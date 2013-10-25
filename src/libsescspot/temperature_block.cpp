#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "temperature_block.h"
#include "flp.h"
#include "util.h"

/* 
 * allocate memory for the matrices. placeholder can be an empty 
 * floorplan frame with only the names of the functional units
 */
block_model_t *alloc_block_model(thermal_config_t *config, flp_t *placeholder)
{
	/* shortcuts	*/
	int32_t n = placeholder->n_units;
	int32_t m = NL*n+EXTRA;

	block_model_t *model = (block_model_t *) calloc (1, sizeof(block_model_t));
	if (!model)
		fatal("memory allocation error\n");
	model->config = *config;
	model->n_units = model->base_n_units = n;
	model->n_nodes = m;

	model->border = imatrix(n, 4);
	model->len = dmatrix(n, n);	/* len[i][j] = length of shared edge bet. i & j	*/
	model->g = dmatrix(m, m);	/* g[i][j] = conductance bet. nodes i & j */
	model->gx = dvector(n);		/* lumped conductances in x direction	*/
	model->gy = dvector(n);		/* lumped conductances in y direction	*/
	model->gx_sp = dvector(n);	/* lateral conductances in the spreader	layer */
	model->gy_sp = dvector(n);
	model->t_vector = dvector(m);/* scratch pad	*/
	model->p = ivector(m);		/* permutation vector for b's LUP decomposition	*/

	model->a = dvector(m);		/* vertical Cs - diagonal matrix stored as a 1-d vector	*/
	model->inva = dvector(m);	/* inverse of the above 	*/
	/* B, C and LU are (NL*n+EXTRA)x(NL*n+EXTRA) matrices	*/
	model->b = dmatrix(m, m);
	model->c = dmatrix(m, m);
	model->lu = dmatrix(m, m);
	model->factor_pack = C_FACTOR;

	model->flp = placeholder;
	return model;
}

/* creates matrices  B and invB: BT = Power in the steady state. 
 * NOTE: EXTRA nodes: 1 interface bottom, 5 spreader and 5 heat sink nodes
 * (north, south, east, west and bottom).
 */
void populate_R_model_block(block_model_t *model, flp_t *flp)
{
	/*	shortcuts	*/
	double **b = model->b;
	double *gx = model->gx, *gy = model->gy;
	double *gx_sp = model->gx_sp, *gy_sp = model->gy_sp;
	double **len = model->len, **g = model->g, **lu = model->lu;
	int32_t **border = model->border;
	int32_t *p = model->p;
	double t_chip = model->config.t_chip;
	double r_convec = model->config.r_convec;
	double s_sink = model->config.s_sink;
	double t_sink = model->config.t_sink;
	double s_spreader = model->config.s_spreader;
	double t_spreader = model->config.t_spreader;
	double t_interface = model->config.t_interface;

	int32_t i, j, k = 0, n = flp->n_units;
	double r_sp1, r_sp2, r_hs;	/* lateral resistances to spreader and heatsink	*/

	/* NOTE: *_mid - the vertical Rs from CENTER nodes of spreader 
	 * and heatsink. *_per - the vertical Rs from PERIPHERAL (n,s,e,w) nodes
	 */
	double r_sp_per, r_hs_mid, r_hs_per;
	double gn_sp=0, gs_sp=0, ge_sp=0, gw_sp=0;

	double w_chip = get_total_width (flp);	/* x-axis	*/
	double l_chip = get_total_height (flp);	/* y-axis	*/

	/* sanity check on floorplan sizes	*/
	if (w_chip > s_sink || l_chip > s_sink || 
		w_chip > s_spreader || l_chip > s_spreader) {
		print_flp(flp);
		print_flp_fig(flp);
		fatal("inordinate floorplan size!\n");
	}
	if(model->flp != flp || model->n_units != flp->n_units ||
	   model->n_nodes != NL * flp->n_units + EXTRA)
	   fatal("mismatch between the floorplan and the thermal model\n");

	/* gx's and gy's of blocks	*/
	for (i = 0; i < n; i++) {
		/* at the silicon layer	*/
		if (model->config.block_omit_lateral) {
			gx[i] = gy[i] = 0;
		}
		else {
			gx[i] = 1.0/getr(K_SI, flp->units[i].height, flp->units[i].width, l_chip, t_chip);
			gy[i] = 1.0/getr(K_SI, flp->units[i].width, flp->units[i].height, w_chip, t_chip);
		}

		/* at the spreader layer	*/
		gx_sp[i] = 1.0/getr(K_CU, flp->units[i].height, flp->units[i].width, l_chip, t_spreader);
		gy_sp[i] = 1.0/getr(K_CU, flp->units[i].width, flp->units[i].height, w_chip, t_spreader);
	}

	/* shared lengths between blocks	*/
	for (i = 0; i < n; i++) 
		for (j = i; j < n; j++) 
			len[i][j] = len[j][i] = get_shared_len(flp, i, j);

	/* lateral R's of spreader and sink */
	r_sp1 = getr(K_CU, (s_spreader+3*w_chip)/4.0, (s_spreader-w_chip)/4.0, w_chip, t_spreader);
	r_sp2 = getr(K_CU, (3*s_spreader+w_chip)/4.0, (s_spreader-w_chip)/4.0, (s_spreader+3*w_chip)/4.0, t_spreader);
	r_hs = getr(K_CU, (s_sink+3*s_spreader)/4.0, (s_sink-s_spreader)/4.0, s_spreader, t_sink);

	/* vertical R's of spreader and sink */
	r_sp_per = RHO_CU * t_spreader * 4.0 / (s_spreader * s_spreader - w_chip*l_chip);
	r_hs_mid = RHO_CU * t_sink / (s_spreader*s_spreader);
	r_hs_per = RHO_CU * t_sink * 4.0 / (s_sink * s_sink - s_spreader*s_spreader);

	/* short the R's from block centers to a particular chip edge	*/
	for (i = 0; i < n; i++) {
		if (eq(flp->units[i].bottomy + flp->units[i].height, l_chip)) {
			gn_sp += gy_sp[i];
			border[i][2] = 1;	/* block is on northern border 	*/
		} else
			border[i][2] = 0;

		if (eq(flp->units[i].bottomy, 0)) {
			gs_sp += gy_sp[i];
			border[i][3] = 1;	/* block is on southern border	*/
		} else
			border[i][3] = 0;

		if (eq(flp->units[i].leftx + flp->units[i].width, w_chip)) {
			ge_sp += gx_sp[i];
			border[i][1] = 1;	/* block is on eastern border	*/
		} else 
			border[i][1] = 0;

		if (eq(flp->units[i].leftx, 0)) {
			gw_sp += gx_sp[i];
			border[i][0] = 1;	/* block is on western border	*/
		} else
			border[i][0] = 0;
	}

	/* initialize g	*/
	zero_dmatrix(g, NL*n+EXTRA, NL*n+EXTRA);

	/* overall Rs between nodes */
	for (i = 0; i < n; i++) {
		double area = (flp->units[i].height * flp->units[i].width);
		/* amongst functional units	in the various layers	*/
		for (j = 0; j < n; j++) {
			double part = 0, part_sp = 0;
			if (is_horiz_adj(flp, i, j)) {
				part = gx[i] / flp->units[i].height;
				part_sp = gx_sp[i] / flp->units[i].height;
			}
			else if (is_vert_adj(flp, i,j))  {
				part = gy[i] / flp->units[i].width;
				part_sp = gy_sp[i] / flp->units[i].width;
			}
			g[i][j] = part * len[i][j];
		 	/* resistances in the interface layer are assumed 
		 	 * to be infinite. the corresponding g's are
			 * zero and hence are omitted here.
			 */
			g[HSP*n+i][HSP*n+j] = part_sp * len[i][j];
		}

 		/* vertical g's in the silicon layer	*/
		g[i][IFACE*n+i]=g[IFACE*n+i][i]=2.0/(RHO_SI * t_chip / area);
 		/* vertical g's in the interface layer	*/
		g[IFACE*n+i][HSP*n+i]=g[HSP*n+i][IFACE*n+i]=2.0/(RHO_INT * t_interface / area);
		/* vertical g's in the spreader layer	*/
		g[HSP*n+i][NL*n+SP_B]=g[NL*n+SP_B][HSP*n+i]=2.0/(RHO_CU * t_spreader / area);

		/* lateral g's from block center (spreader layer) to peripheral (n,s,e,w) spreader nodes	*/
		g[HSP*n+i][NL*n+SP_N]=g[NL*n+SP_N][HSP*n+i]=2.0*border[i][2]/((1.0/gy_sp[i])+r_sp1*gn_sp/gy_sp[i]);
		g[HSP*n+i][NL*n+SP_S]=g[NL*n+SP_S][HSP*n+i]=2.0*border[i][3]/((1.0/gy_sp[i])+r_sp1*gs_sp/gy_sp[i]);
		g[HSP*n+i][NL*n+SP_E]=g[NL*n+SP_E][HSP*n+i]=2.0*border[i][1]/((1.0/gx_sp[i])+r_sp1*ge_sp/gx_sp[i]);
		g[HSP*n+i][NL*n+SP_W]=g[NL*n+SP_W][HSP*n+i]=2.0*border[i][0]/((1.0/gx_sp[i])+r_sp1*gw_sp/gx_sp[i]);
	}

	/* vertical g's between central nodes	*/
 	/* between spreader bottom and sink bottom	*/
	g[NL*n+SINK_B][NL*n+SP_B]=g[NL*n+SP_B][NL*n+SINK_B]=2.0/r_hs_mid;

	/* g's from peripheral(n,s,e,w) nodes	*/
	for (i = 1; i <= 4; i++) {
 		/* vertical g's between peripheral spreader nodes and spreader bottom */
		g[NL*n+SP_B-i][NL*n+SP_B]=g[NL*n+SP_B][NL*n+SP_B-i]=2.0/r_sp_per;
 		/* lateral g's between peripheral spreader nodes and peripheral sink nodes	*/
		g[NL*n+SP_B-i][NL*n+SINK_B-i]=g[NL*n+SINK_B-i][NL*n+SP_B-i]=2.0/(r_hs + r_sp2);
 		/* vertical g's between peripheral sink nodes and sink bottom	*/
		g[NL*n+SINK_B-i][NL*n+SINK_B]=g[NL*n+SINK_B][NL*n+SINK_B-i]=2.0/r_hs_per;
	}	

	/* calculate matrix B such that BT = POWER in steady state */
	for (i = 0; i < NL*n+EXTRA; i++) {
		for (j = 0; j < NL*n+EXTRA; j++) {
			if (i==j) {
				if (i == NL*n+SINK_B)	/* sink bottom */
					b[i][j] = 1.0 / r_convec;
				else
					b[i][j] = 0;
				for (k = 0; k < NL*n+EXTRA; k++) {
					if ((g[i][k]==0.0)||(g[k][i])==0.0) 
						continue;
					else 
					/* here is why the 2.0 factor comes when calculating g[][]	*/
						b[i][j] += 1.0/((1.0/g[i][k])+(1.0/g[k][i]));
				}
			} else {
				if ((g[i][j]==0.0)||(g[j][i])==0.0)
					b[i][j]=0.0;
				else
					b[i][j]=-1.0/((1.0/g[i][j])+(1.0/g[j][i]));
			}
		}
	}

	/* compute the LUP decomposition of B and store it too	*/
	copy_dmatrix(lu, b, NL*n+EXTRA, NL*n+EXTRA);
	/* 
	 * B is a symmetric positive definite matrix. It is
	 * symmetric because if a node A is connected to B, 
	 * then B is also connected to A with the same R value.
	 * It is positive definite because of the following
	 * informal argument from Professor Lieven Vandenberghe's
	 * lecture slides for the spring 2004-2005 EE 103 class 
	 * at UCLA: http://www.ee.ucla.edu/~vandenbe/103/chol.pdf
	 * x^T*B*x = voltage^T * (B*x) = voltage^T * current
	 * = total power dissipated in the resistors > 0 
	 * for x != 0. 
	 */
	lupdcmp(lu, NL*n+EXTRA, p, 1);

	/* done	*/
	model->flp = flp;
	model->r_ready = TRUE;
}

/* creates 2 matrices: invA, C: dT + A^-1*BT = A^-1*Power, 
 * C = A^-1 * B. note that A is a diagonal matrix (no lateral
 * capacitances. all capacitances are to ground). also note that
 * it is stored as a 1-d vector. so, for computing the inverse, 
 * inva[i] = 1/a[i] is just enough. NOTE: EXTRA nodes: 1 interface 
 * bottom, 5 spreader and 5 heat sink nodes (north, south, east, 
 * west and bottom).
 */

void populate_C_model_block(block_model_t *model, flp_t *flp)
{
	/*	shortcuts	*/
	double *inva = model->inva, **c = model->c;
	double **b = model->b;
	double *a = model->a;
	double t_chip = model->config.t_chip;
	double c_convec = model->config.c_convec;
	double s_sink = model->config.s_sink;
	double t_sink = model->config.t_sink;
	double s_spreader = model->config.s_spreader;
	double t_spreader = model->config.t_spreader;
	double t_interface = model->config.t_interface;

	int32_t i, n = flp->n_units;

	if (!model->r_ready)
		fatal("R model not ready\n");
	if (model->flp != flp || model->n_units != flp->n_units ||
		model->n_nodes != NL * flp->n_units + EXTRA)
		fatal("different floorplans for R and C models!");
		

	/* NOTE: *_mid - the vertical Cs from CENTER nodes of spreader 
	 * and heatsink. *_per - the vertical Cs from PERIPHERAL (n,s,e,w) nodes
	 */
	double  c_sp_per, c_hs_mid, c_hs_per;

	double w_chip = get_total_width (flp);	/* x-axis	*/
	double l_chip = get_total_height (flp);	/* y-axis	*/

	/* compute the silicon fitting factor - see pg 10 of the UVA CS tech report - CS-TR-2003-08	*/
	model->factor_chip = C_FACTOR * ((SPEC_HEAT_INT / SPEC_HEAT_SI) * (w_chip + 0.88 * t_interface) \
				* (l_chip + 0.88 * t_interface) * t_interface / ( w_chip * l_chip * t_chip) + 1);

	/* fitting factor for interface	 - same rationale as above */
	model->factor_inter = C_FACTOR * ((SPEC_HEAT_CU / SPEC_HEAT_INT) * (w_chip + 0.88 * t_spreader) \
				* (l_chip + 0.88 * t_spreader) * t_spreader / ( w_chip * l_chip * t_interface) + 1);

	/*fprintf(stdout, "fitting factors : %f, %f\n", factor_chip, factor_inter);	*/

	/* vertical C's of spreader and sink */
	c_sp_per = model->factor_pack * SPEC_HEAT_CU * t_spreader * (s_spreader * s_spreader - w_chip*l_chip) / 4.0;
	c_hs_mid = model->factor_pack * SPEC_HEAT_CU * t_sink * (s_spreader * s_spreader);
	c_hs_per = model->factor_pack * SPEC_HEAT_CU * t_sink * (s_sink * s_sink - s_spreader*s_spreader) / 4.0;

	/* overall Cs between nodes */
	for (i = 0; i < n; i++) {
		double area = (flp->units[i].height * flp->units[i].width);
		/* C's from functional units to ground	*/
		a[i] = model->factor_chip * SPEC_HEAT_SI * t_chip * area;
		/* C's from interface portion of the functional units to ground	*/
		a[IFACE*n+i] = model->factor_inter * SPEC_HEAT_INT * t_interface * area;
		/* C's from spreader portion of the functional units to ground	*/
		a[HSP*n+i] = model->factor_pack * SPEC_HEAT_CU * t_spreader * area;
	}

	/* vertical C's between central nodes	*/
 	/* from spreader bottom to ground	*/
	a[NL*n+SP_B] = c_hs_mid;
 	/* from sink bottom to ground	*/
	a[NL*n+SINK_B] = model->factor_pack * c_convec;

	/* C's from peripheral(n,s,e,w) nodes	*/
	for (i = 1; i <= 4; i++) {
 		/* from peripheral spreader nodes to ground	*/
		a[NL*n+SP_B-i] = c_sp_per;
 		/* from peripheral sink nodes to ground	*/
		a[NL*n+SINK_B-i] = c_hs_per;
	}

	/* calculate A^-1 (for diagonal matrix A) such that A(dT) + BT = POWER */
	for (i = 0; i < NL*n+EXTRA; i++)
		inva[i] = 1.0/a[i];

	/* we are always going to use the eqn dT + A^-1 * B T = A^-1 * POWER. so, store  C = A^-1 * B	*/
	diagmatmult(c, inva, b, NL*n+EXTRA);

	/*	done	*/
	model->c_ready = TRUE;
}

/* setting internal node power numbers	*/
void set_internal_power_block (block_model_t *model, double *pow)
{
	zero_dvector(&pow[model->n_units], (model->n_nodes-model->n_units-1));
	pow[model->n_nodes-1] = model->config.ambient / model->config.r_convec;
}

/* power and temp should both be alloced using hotspot_vector. 
 * 'b' is the 'thermal conductance' matrix. i.e, b * temp = power
 *  => temp = invb * power. instead of computing invb, we have
 * stored the LUP decomposition of B in 'lu' and 'p'. Using
 * forward and backward substitution, we can then solve the 
 * equation b * temp = power.
 */
void steady_state_temp_block(block_model_t *model, double *power, double *temp) 
{
	if (!model->r_ready)
		fatal("R model not ready\n");

	/* set power numbers for the virtual nodes */
	set_internal_power_block(model, power);

	/* 
	 * find temperatures (spd flag is set to 1 by the same argument
	 * as mentioned in the populate_R_model_block function)
	 */
	lusolve(model->lu, model->n_nodes, model->p, power, temp, 1);
}

/* compute_temp: solve for temperature from the equation dT + CT = inv_A * Power 
 * Given the temperature (temp) at time t, the power dissipation per cycle during the 
 * last interval (time_elapsed), find the new temperature at time t+time_elapsed.
 * power and temp should both be alloced using hotspot_vector
 */
void compute_temp_block(block_model_t *model, double *power, double *temp, double time_elapsed)
{
	double t, h, new_h;
	
	if (!model->r_ready || !model->c_ready)
		fatal("block model not ready\n");
	if (temp == model->t_vector)
		fatal("output same as scratch pad\n");

	/* set power numbers for the virtual nodes */
	set_internal_power_block(model, power);

	/* use the scratch pad vector to find (inv_A)*POWER */
	diagmatvectmult(model->t_vector, model->inva, power, model->n_nodes);

	/* Obtain temp at time (t+time_elapsed). 
	 * Instead of getting the temperature at t+time_elapsed directly, we do it 
	 * in multiple steps with the correct step size at each time 
	 * provided by rk4
	 */
	for (t = 0, new_h = MIN_STEP; t + new_h <= time_elapsed; t+=h) {
		h = new_h;
		new_h = rk4(model->c, temp, model->t_vector, model->n_nodes, h, temp);
	}
	/* remainder	*/
	if (time_elapsed > t)
		rk4(model->c, temp, model->t_vector, model->n_nodes, time_elapsed - t, temp);
}

/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector_block(block_model_t *model)
{
	return dvector(model->n_nodes);
}

/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector_block(block_model_t *model, double *dst, double *src, 
						 	   int32_t at, int32_t size)
{
	int32_t i;

	for (i=0; i < at && i < model->n_nodes; i++)
		dst[i] = src[i];
	for(i=at+size; i < model->n_nodes; i++)
		dst[i-size] = src[i];
}

/* update the model's node count	*/						 
void resize_thermal_model_block(block_model_t *model, int32_t n_units)
{
	if (n_units > model->base_n_units)
		fatal("resizing block model to more than the allocated space\n");
	model->n_units = n_units;
	model->n_nodes = NL * n_units + EXTRA;
	/* resize the 2-d matrices whose no. of columns changes	*/
	resize_dmatrix(model->len, model->n_units, model->n_units);
	resize_dmatrix(model->g, model->n_nodes, model->n_nodes);
	resize_dmatrix(model->b, model->n_nodes, model->n_nodes);
	resize_dmatrix(model->c, model->n_nodes, model->n_nodes);
	resize_dmatrix(model->lu, model->n_nodes, model->n_nodes);
}

/* sets the temperature of a vector 'temp' allocated using 'hotspot_vector'	*/
void set_temp_block(block_model_t *model, double *temp, double val)
{
	int32_t i;
	for(i=0; i < model->n_nodes; i++)
		temp[i] = val;
}

/* dump temperature vector alloced using 'hotspot_vector' to 'file' */ 
void dump_temp_block(block_model_t *model, double *temp, char *file)
{
	flp_t *flp = model->flp;
	int32_t i;
	char str[STR_SIZE];
	FILE *fp;

	if (!strcasecmp(file, "stdout"))
		fp = stdout;
	else if (!strcasecmp(file, "stderr"))
		fp = stderr;
	else 	
		fp = fopen (file, "w");

	if (!fp) {
		sprintf (str,"error: %s could not be opened for writing\n", file);
		fatal(str);
	}
	/* on chip temperatures	*/
	for (i=0; i < flp->n_units; i++)
		fprintf(fp, "%s\t%.1f\n", flp->units[i].name, temp[i]);

	/* interface temperatures	*/
	for (i=0; i < flp->n_units; i++)
		fprintf(fp, "iface_%s\t%.1f\n", flp->units[i].name, temp[IFACE*flp->n_units+i]);

	/* spreader temperatures	*/
	for (i=0; i < flp->n_units; i++)
		fprintf(fp, "hsp_%s\t%.1f\n", flp->units[i].name, temp[HSP*flp->n_units+i]);

	/* internal node temperatures	*/
	for (i=0; i < EXTRA; i++) {
		sprintf(str, "inode_%d", i);
		fprintf(fp, "%s\t%.1f\n", str, temp[i+NL*flp->n_units]);
	}

	if(fp != stdout && fp != stderr)
		fclose(fp);	
}

/* 
 * read temperature vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_temp'. values are clipped to thermal
 * threshold based on 'clip'
 */ 
void read_temp_block(block_model_t *model, double *temp, char *file, int32_t clip)
{
	/*	shortcuts	*/
	flp_t *flp = model->flp;
	double thermal_threshold = model->config.thermal_threshold;
	double ambient = model->config.ambient;

	int32_t i, idx;
	double max=0, val;
	char *ptr, str1[STR_SIZE], str2[STR_SIZE], name[STR_SIZE];
	FILE *fp;

	if (!strcasecmp(file, "stdin"))
		fp = stdin;
	else
		fp = fopen (file, "r");

	if (!fp) {
		sprintf (str1,"error: %s could not be opened for reading\n", file);
		fatal(str1);
	}	

	/* find max temp on the chip	*/
	for (i=0; i < flp->n_units; i++) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			fatal("not enough lines in temperature file\n");
		strcpy(str2, str1);
		ptr = strtok(str1, " \t\n");
		if (ptr && ptr[0] != '#') {	/* ignore comments and empty lines	*/
		  if (sscanf(str2, "%s%lf", name, &val) != 2)
			fatal("invalid temperature file format\n");
		  idx = get_blk_index(flp, name);
		  if (idx >= 0)
			temp[idx] = val;
		  else	/* since get_blk_index calls fatal, the line below cannot be reached	*/
			fatal ("unit in temperature file not found in floorplan\n");
		  if (temp[idx] > max)
			max = temp[idx];
		}	
	}

	/* interface material temperatures	*/
	for (i=0; i < flp->n_units; i++) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			fatal("not enough lines in temperature file\n");
		strcpy(str2, str1);
		if (sscanf(str2, "iface_%s%lf", name, &val) != 2)
			fatal("invalid temperature file format\n");
		idx = get_blk_index(flp, name);
		if (idx >= 0)
			temp[idx+IFACE*flp->n_units] = val;
		else	/* since get_blk_index calls fatal, the line below cannot be reached	*/
			fatal ("unit in temperature file not found in floorplan\n");
	}

	/* heat spreader temperatures	*/
	for (i=0; i < flp->n_units; i++) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			fatal("not enough lines in temperature file\n");
		strcpy(str2, str1);
		if (sscanf(str2, "hsp_%s%lf", name, &val) != 2)
			fatal("invalid temperature file format\n");
		idx = get_blk_index(flp, name);
		if (idx >= 0)
			temp[idx+HSP*flp->n_units] = val;
		else	/* since get_blk_index calls fatal, the line below cannot be reached	*/
			fatal ("unit in temperature file not found in floorplan\n");
	}

	/* internal node temperatures	*/	
	for (i=0; i < EXTRA; i++) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			fatal("not enough lines in temperature file\n");
		strcpy(str2, str1);
		if (sscanf(str2, "%s%lf", name, &val) != 2)
			fatal("invalid temperature file format\n");
		sprintf(str1, "inode_%d", i);
		if (strcasecmp(str1, name))
			fatal("invalid temperature file format\n");
		temp[i+NL*flp->n_units] = val;	
	}

	if(fp != stdin)
		fclose(fp);	

	/* clipping	*/
	if (clip && (max > thermal_threshold)) {
		/* if max has to be brought down to thermal_threshold, 
		 * (w.r.t the ambient) what is the scale down factor?
		 */
		double factor = (thermal_threshold - ambient) / (max - ambient);
	
		/* scale down all temperature differences (from ambient) by the same factor	*/
		for (i=0; i < NL*flp->n_units + EXTRA; i++)
			temp[i] = (temp[i]-ambient)*factor + ambient;
	}
}

/* dump power numbers to file	*/
void dump_power_block(block_model_t *model, double *power, char *file)
{
	flp_t *flp = model->flp;
	int32_t i;
	char str[STR_SIZE];
	FILE *fp;

	if (!strcasecmp(file, "stdout"))
		fp = stdout;
	else if (!strcasecmp(file, "stderr"))
		fp = stderr;
	else 	
		fp = fopen (file, "w");

	if (!fp) {
		sprintf (str,"error: %s could not be opened for writing\n", file);
		fatal(str);
	}
	for (i=0; i < flp->n_units; i++)
		fprintf(fp, "%s\t%.3f\n", flp->units[i].name, power[i]);
	if(fp != stdout && fp != stderr)
		fclose(fp);	
}

/* 
 * read power vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_power'. 
 */ 
void read_power_block (block_model_t *model, double *power, char *file)
{
	flp_t *flp = model->flp;
	int32_t i=0, idx;
	double val;
	char *ptr, str1[STR_SIZE], str2[STR_SIZE], name[STR_SIZE];
	FILE *fp;

	if (!strcasecmp(file, "stdin"))
		fp = stdin;
	else
		fp = fopen (file, "r");

	if (!fp) {
		sprintf (str1,"error: %s could not be opened for reading\n", file);
		fatal(str1);
	}
	while(!feof(fp)) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			break;
		strcpy(str2, str1);
		
		ptr = strtok(str1, " \t\n");
		if (ptr && ptr[0] != '#') {	/* ignore comments and empty lines	*/
		  if (sscanf(str2, "%s%lf", name, &val) != 2)
			fatal("invalid power file format\n");
		  idx = get_blk_index(flp, name);
		  if (idx >= 0)
			power[idx] = val;
		  else	/* since get_blk_index calls fatal, the line below cannot be reached	*/
			fatal ("unit in power file not found in floorplan\n");
		  i++;
		}
	}
	if(fp != stdin)
		fclose(fp);
}

double find_max_temp_block(block_model_t *model, double *temp)
{
	int32_t i;
	double max = 0.0;
	for(i=0; i < model->n_units; i++) {
		if (temp[i] < 0)
			fatal("negative temperature!");
		else if (max < temp[i])
			max = temp[i];
	}

	return max;
}

double find_avg_temp_block(block_model_t *model, double *temp)
{
	int32_t i;
	double sum = 0.0;
	for(i=0; i < model->n_units; i++) {
		if (temp[i] < 0)
			fatal("negative temperature!");
		else 
			sum += temp[i];
	}

	return (sum / model->n_units);
}

void delete_block_model(block_model_t *model)
{
	free_dvector(model->a);
	free_dvector(model->inva);
	free_dmatrix(model->b);
	free_dmatrix(model->c);

	free_dvector(model->gx);
	free_dvector(model->gy);
	free_dvector(model->gx_sp);
	free_dvector(model->gy_sp);
	free_dvector(model->t_vector);
	free_ivector(model->p);

	free_dmatrix(model->len);
	free_dmatrix(model->g);
	free_dmatrix(model->lu);

	free_imatrix(model->border);

	free(model);
}
