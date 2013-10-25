#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>

#include "temperature_grid.h"
#include "flp.h"
#include "util.h"
#include "FBlocks.h"
#include "Grids.h"

/* constructor	*/ 
grid_model_t *alloc_grid_model(thermal_config_t *config, flp_t *flp_default)
{
	grid_model_t *model = new grid_model_t;
	if (!model)
		fatal("memory allocation error\n");
	model->flp = flp_default;
	model->chip_width = get_total_width(flp_default);
	model->chip_height = get_total_height(flp_default);
	model->config = *config;
	model->layer_cfg = config->grid_layer_file;
	model->Tamb = config->ambient;
	model->row = config->grid_rows;
	model->col = config->grid_cols;
	model->mv_level = 0;

	if(!strcasecmp(model->config.grid_map_mode, GRID_AVG_STR))
		model->map_mode = GRID_AVG;
	else if(!strcasecmp(model->config.grid_map_mode, GRID_MIN_STR))
		model->map_mode = GRID_MIN;
	else if(!strcasecmp(model->config.grid_map_mode, GRID_MAX_STR))
		model->map_mode = GRID_MAX;
	else if(!strcasecmp(model->config.grid_map_mode, GRID_CENTER_STR))
		model->map_mode = GRID_CENTER;
	else
		fatal("unknown mapping mode\n");

	return model;
}

/* thermal resistances only	*/
void populate_R_model_grid(grid_model_t *model)
{
	int32_t layer_total = layer_R_setup(model);
	init_gpower(layer_total, model); /* assign zero intial power to all grid cells */
	  
	/* set up grid cells, and return the number of nodes excluding 
	 * those of the heatsink. the number of active nodes dissipating
	 * power (i.e., sum of all the functional blocks of all the floorplans)
	 * is stored in model->sum_n_units.
	 */
	model->total_n_blocks = grid_setup(model, model->config.init_temp);
	/* done	*/
	model->r_ready = TRUE;
}

/* thermal capacitances	*/
void populate_C_model_grid(grid_model_t *model)
{
	if (!model->r_ready)
		fatal("R model not ready\n");

	layer_C_setup(model);
	/* done	*/
	model->c_ready = TRUE;
}

/* compute the steady temperatures by iteratively solving the 
 * equations. 
 */
void steady_state_temp_grid(grid_model_t *model, double *power, double *temp) 
{
	if (!model->r_ready)
		fatal("R model not ready\n");
		
	int32_t i;		
	vector<double> steady_temp_g, overall_power_g;	
	
	/* allocate and initialize temperature and power vectors */	
	steady_temp_g.resize(model->total_n_blocks+EXTRA, model->config.init_temp);
	overall_power_g.resize(model->total_n_blocks+EXTRA, 0.0);
	
	/*	copy "hotspot vectors" to C++ vectors	*/
	for (i=0; i<model->total_n_blocks+EXTRA; i++) {
		steady_temp_g[i]=temp[i];
		overall_power_g[i]=power[i];
	}
	
	/* mapping block powers into grid powers */
	trans_bpow_gpow (overall_power_g, model);
	
	/* call steady-state solver */
  	fast_steady_solver(model);
  
	/* mapping static grid temperatures to block temperatures */		 		
	trans_steady_gtemp_btemp(steady_temp_g, model);
	
	/*	copy results back to "hotspot vector"	*/
	for (i=0; i<model->total_n_blocks+EXTRA; i++) {
		temp[i]=steady_temp_g[i];
	}

	/* also dump the internal grid temperatures if specified	*/
	if(strcmp(model->config.grid_steady_file, NULLFILE)) {
		ofstream fout_result (model->config.grid_steady_file);
		for (int32_t i=0; i<(int)(model->row)*(model->col); i++)
			fout_result << i << "\t" << model->steady_g_temp[i] << endl;
		fout_result.close();
	}
}

/* compute the transient temperatures by iteratively solving
 * the difference equations. 
 */
void compute_temp_grid(grid_model_t *model, double *power, double *temp, double time_elapsed)
{
	int32_t i;
	
	if (!model->r_ready || !model->c_ready)
		fatal("grid model not ready\n");

	vector<double> temp_g, power_g;
	
	/* allocate and initialize temperature and power vectors */	
	temp_g.resize(model->total_n_blocks+EXTRA, model->config.init_temp);
	power_g.resize(model->total_n_blocks+EXTRA, 0.0);

	/*	copy "hotspot vectors" to C++ vectors	*/	
	for (i=0; i<model->total_n_blocks+EXTRA; i++) {
		temp_g[i]=temp[i];
		power_g[i]=power[i];
	}
	
	/* mapping block powers into grid powers */
	trans_bpow_gpow (power_g, model);
	
	/* mapping initial block temp to grid temp */
	trans_tran_btemp_gtemp(temp_g, model);
	
	/* call steady-state solver */
  	fast_tran_solver(time_elapsed, model);
  
	/* mapping static grid temperatures to block temperatures */		 		
	trans_tran_gtemp_btemp(temp_g, model);

	/*	copy results back to "hotspot vector"	*/	
	for (i=0; i<model->total_n_blocks+EXTRA; i++) {
		temp[i]=temp_g[i];
	}
}

/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector_grid(grid_model_t *model)
{
	if (model->total_n_blocks <= 0)
		fatal("total_n_blocks is not greater than zero\n");
	return dvector(model->total_n_blocks + EXTRA);
}

/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector_grid(grid_model_t *model, double *dst, double *src, 
						 	  int32_t at, int32_t size)
{
	int32_t i;
	for (i=0; i < at && i < model->total_n_blocks + EXTRA; i++)
		dst[i] = src[i];
	for(i=at+size; i < model->total_n_blocks + EXTRA; i++)
		dst[i-size] = src[i];
}

/* sets the temperature of a vector 'temp' allocated using 'hotspot_vector'	*/
void set_temp_grid(grid_model_t *model, double *temp, double val)
{
	int32_t i;
	if (model->total_n_blocks <= 0)
		fatal("total_n_blocks is not greater than zero\n");
	for(i=0; i < model->total_n_blocks + EXTRA; i++)
		temp[i] = val;
}

/* dump temperature vector of all layers to 'file' */ 
void dump_temp_grid(grid_model_t *model, double *temp, char *file)
{
	vector<flp_t *> layer_flp;
	flp_t *flp;
	int32_t i;
	uint32_t k;
	int32_t count=0;
	char str[STR_SIZE];

	/* precondition: layer config file should have been read	*/
	if (!model->r_ready)
		fatal("R model not ready. call alloc_R_model before this\n");
	
	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
		layer_flp.push_back(flp);
	}
	
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
	
	for (k=0; k<model->Layers[0].size(); k++) { 
		/* on chip temperatures	*/
		if (model->Layers[0][k].Power) { /* active silicon layer */
			for (i=0; i < layer_flp[k]->n_units; i++)
				fprintf(fp, "%s\t%.1f\n", layer_flp[k]->units[i].name, temp[i+count]);
		}
		else if (k==model->Layers[0].size()-1) {/* spreader center temperatures	*/
			for (i=0; i < layer_flp[k]->n_units; i++)
				fprintf(fp, "hsp_%s\t%.1f\n", layer_flp[k]->units[i].name, temp[count+i]);
		}
		else { /* inter-layer dioxide or dielectric or TIM */
			for (i=0; i < layer_flp[k]->n_units; i++)
				fprintf(fp, "iface_%s\t%.1f\n", layer_flp[k]->units[i].name, temp[count+i]);
		}
		count += layer_flp[k]->n_units;
	}
  	
	/* internal node temperatures	*/
	for (i=0; i < EXTRA; i++) {
		sprintf(str, "inode_%d", i);
		fprintf(fp, "%s\t%.1f\n", str, temp[i+count]);
	}
	if(fp != stdout && fp != stderr)
		fclose(fp);	

	for(k=0; k < model->Layers[0].size(); k++)
		free_flp(layer_flp[k], 0);
}

/* 
 * read temperature vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_temp'. values are clipped to thermal
 * threshold based on 'clip'.
 */ 
void read_temp_grid(grid_model_t *model, double *temp, char *file, int32_t clip)
{
	/*	shortcuts	*/
	double thermal_threshold = model->config.thermal_threshold;
	double ambient = model->config.ambient;
  
  	int32_t i;
  	double max=0.0, val;
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
	/* names are no longer checked against the floorplan in this release	*/
	for (i=0; i < model->total_n_blocks+EXTRA; i++) {
		fgets(str1, STR_SIZE, fp);
		if (feof(fp))
			fatal("not enough lines in temperature file\n");
		strcpy(str2, str1);	
		ptr = strtok(str1, " \t\n");
		if (ptr && ptr[0] != '#') {	/* ignore comments and empty lines	*/
		  if (sscanf(str2, "%s%lf", name, &val) != 2)
			fatal("invalid temperature file format\n");
		  temp[i] = val;
		  if (temp[i] > max)
			max = temp[i];
		}	
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
		for (i=0; i < model->total_n_blocks+EXTRA; i++)
			temp[i] = (temp[i]-ambient)*factor + ambient;
	}
}

/* dump power numbers to file	*/
void dump_power_grid(grid_model_t *model, double *power, char *file)
{
	int32_t i;
	char str[STR_SIZE];
	
	uint32_t k, count=0;
  	vector<flp_t *> layer_flp;
	flp_t *flp;
	
	/* precondition: layer config file should have been read	*/
	if (!model->r_ready)
		fatal("R model not ready. call alloc_R_model before this\n");
	
  	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
		layer_flp.push_back(flp);
	}
	
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
	
  	/* only dump functional blocks' powers in active layers */
  	for (k=0; k<model->Layers[0].size(); k++) {
  		if (model->Layers[0][k].Power) {
			for (i=0; i < layer_flp[k]->n_units; i++)
				fprintf(fp, "%s\t%.3f\n", layer_flp[k]->units[i].name, power[i+count]);
			count += layer_flp[k]->n_units;
		}
	}
	if(fp != stdout && fp != stderr)
		fclose(fp);	

	for(k=0; k < model->Layers[0].size(); k++)
		free_flp(layer_flp[k], 0);
}

/* 
 * read power vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_power'. 
 */ 
void read_power_grid (grid_model_t *model, double *power, char *file)
{
	int32_t i=0;
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
		  power[i] = val;
		  i++;
		}
	}
	if(fp != stdin)
		fclose(fp);
}

double find_max_temp_grid(grid_model_t *model, double *temp)
{
	int32_t i;
	uint32_t k, count=0;
  	vector<flp_t *> layer_flp;
	flp_t *flp;
	double max = 0.0;
	
	/* precondition: layer config file should have been read	*/
	if (!model->r_ready)
		fatal("R model not ready. call alloc_R_model before this\n");
	
  	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
		layer_flp.push_back(flp);
	}

  	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
  		if (model->Layers[0][k].Power) {
			for (i=0; i < layer_flp[k]->n_units; i++) {
				if (temp[i+count] < 0)
					fatal("negative temperature!");
				else if (max < temp[i+count])
					max = temp[i+count];
			}		
			count += layer_flp[k]->n_units;
		}
		free_flp(flp, 0);
	}

	return max;
}

double find_avg_temp_grid(grid_model_t *model, double *temp)
{
	int32_t i, j=0;
	uint32_t k, count=0;
  	vector<flp_t *> layer_flp;
	flp_t *flp;
	double sum = 0.0;

	/* precondition: layer config file should have been read	*/
	if (!model->r_ready)
		fatal("R model not ready. call alloc_R_model before this\n");
	
  	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
		layer_flp.push_back(flp);
	}

  	for (k=0; k<model->Layers[0].size(); k++) { 
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
  		if (model->Layers[0][k].Power) {
			for (i=0; i < layer_flp[k]->n_units; i++) {
				if (temp[i+count] < 0)
					fatal("negative temperature!");
				else {
					sum += temp[i+count];
					j++;
				}	
			}		
			count += layer_flp[k]->n_units;
		}
		free_flp(flp, 0);
	}
	if (!j)
		fatal("no power dissipating units?!\n");

	return (sum / j);
}

void delete_grid_model(grid_model_t *model)
{
	delete model;
}

/* Sets up the RC difference equations for the Grid model */
int32_t layer_R_setup(grid_model_t *model) 
{
		/*	shortcuts	*/
	double r_convec = model->config.r_convec;
	double s_sink = model->config.s_sink;
	double t_sink = model->config.t_sink;
	double s_spreader = model->config.s_spreader;
	double t_spreader = model->config.t_spreader;
	int32_t row = model->row;
	int32_t col = model->col;
	
	double r_sp1, r_sp2, r_hs;	/* lateral resistances to spreader and heatsink	*/

	/* NOTE: *_mid - the vertical R/C from CENTER nodes of spreader 
	 * and heatsink. *_per - the vertical R/C from PERIPHERAL (n,s,e,w) nodes
	 */
	double r_sp_per, r_hs_mid, r_hs_per;
	
	/*	get chip width and height from default floorplan */
	double w_chip = get_total_width(model->flp); /* x-axis */
	double l_chip = get_total_height(model->flp);	/* y-axis */
	
	model->chip_width = w_chip;
	model->chip_height = l_chip;
	
	/* sanity check on floorplan sizes	*/
	if (w_chip > s_sink || l_chip > s_sink || 
		w_chip > s_spreader || l_chip > s_spreader) {
		print_flp(model->flp);
		print_flp_fig(model->flp);
		fatal("inordinate floorplan size!\n");
	}	
	
	double height; 		/* dimensions of a regular grid cell when no multigrid algorithm */
	double width;				
	double grid_area;
	
	/* lateral R's of spreader and sink */
	r_sp1 = getr(K_CU, (s_spreader+3*w_chip)/4.0, (s_spreader-w_chip)/4.0, w_chip, t_spreader);
	r_sp2 = getr(K_CU, (3*s_spreader+w_chip)/4.0, (s_spreader-w_chip)/4.0, (s_spreader+3*w_chip)/4.0, t_spreader);
	r_hs = getr(K_CU, (s_sink+3*s_spreader)/4.0, (s_sink-s_spreader)/4.0, s_spreader, t_sink);

	/* vertical R's and C's of spreader and sink */
	r_sp_per = RHO_CU * t_spreader * 4.0 / (s_spreader * s_spreader - w_chip*l_chip);
	r_hs_mid = RHO_CU * t_sink / (s_spreader*s_spreader);
	r_hs_per = RHO_CU * t_sink * 4.0 / (s_sink * s_sink - s_spreader*s_spreader);


	model->Rconv = r_convec;
	model->Rspley = (col * r_sp1); 
	model->Rsplex = (row * r_sp1); 
	model->Rspl = r_sp2 + r_hs;
	model->Rspv = r_sp_per;
	model->Rsi = r_hs_per;
	model->Rsicv = r_hs_mid;
	
	/*
	 * read the layer config file;
	 * set up properties for each layer accordingly;
	 * set up layer info for each multigrid level.
	 */
	string LayerFile = model->layer_cfg;
	ifstream lcf_in (LayerFile.c_str());

	bool done = false;
	char FileLine[STR_SIZE];			
	int32_t count = 0;
	layerprops *temp_layerprops = NULL;					//Temporary store for layer parameters
	
	/* Read from layer config file and store the parameters in model->LayerInternal */
	while (!done) {
		if (lcf_in.eof()) {
			done = true;
		}
		else {
			lcf_in.getline(FileLine, STR_SIZE, '\n');
			if ((FileLine[0] != '#') && (FileLine[0] != '\0')) { /* ignore empty and commented lines */
				if (count == 0) {
					temp_layerprops = new layerprops;
				}
				count++;
				if (count == 1) { 
					temp_layerprops->Id = atoi(FileLine);
				}
				else if (count == 2) {
					if ( (FileLine[0] == 'Y') || (FileLine[0] == 'y') )
						temp_layerprops->Lateral = true;
					else if ( (FileLine[0] == 'N') || (FileLine[0] == 'n') )
						temp_layerprops->Lateral = false;
				}
				else if (count == 3) {
        	if ( (FileLine[0] == 'Y') || (FileLine[0] == 'y') )
          	temp_layerprops->Power = true;
        	else if ( (FileLine[0] == 'N') || (FileLine[0] == 'n') )
          	temp_layerprops->Power = false;
        }
				else if (count == 4)
					temp_layerprops->SpHtCap = atof(FileLine);
				else if (count == 5) {
					temp_layerprops->Rho = atof(FileLine);
					temp_layerprops->K = 1/temp_layerprops->Rho;
				}
				else if (count == 6) {
					temp_layerprops->Thickness = atof( FileLine);
				}
				else if (count == 7 ) {
						temp_layerprops->flp_name = FileLine;
						model->LayerInternal.push_back(*temp_layerprops);
						delete temp_layerprops;
				}
			}
		}
		count = count % DATA_PER_LAYER;
	}
	lcf_in.close();

	/* add the spreader layer (center part of spreader), which is not in the layer config file */
	temp_layerprops = new layerprops;			

	temp_layerprops->Id = model->LayerInternal.size();
	temp_layerprops->Lateral = true;
	temp_layerprops->Power = false;
	temp_layerprops->SpHtCap = SPEC_HEAT_CU;
	temp_layerprops->Rho = RHO_CU;
	temp_layerprops->K = 1/temp_layerprops->Rho;
	temp_layerprops->Thickness = t_spreader;

	int32_t NumOfLayer = model->LayerInternal.size();
	
	/* use the last floorplan for spreader center */
	temp_layerprops->flp_name = model->LayerInternal[NumOfLayer-1].flp_name; 
	
	model->LayerInternal.push_back(*temp_layerprops);
	delete temp_layerprops;

	/* add the spreader to the total layer number */
	NumOfLayer += 1; 
	
	/* note: mv_level=0 in this release, i.e. no multigrid algorithm */
	for (int32_t lvl =0; lvl <= model->mv_level; lvl++) { 
		model->Layers.push_back(vector <LayerDetails>()); /* allocate layer for each multigrid level */
		height = l_chip / (row * (lvl+1));	/* grid size for current multigrid level */
		width = w_chip / (col * (lvl+1));  
		grid_area = height * width;
	
		LayerDetails * tempLayer;
  	
		//Calculate the properties of each layer and store them in Layers
		for (int32_t i = 0; i < NumOfLayer; i++) {
  	
			tempLayer = new LayerDetails;
  	
			tempLayer->Id = i;
  	  tempLayer->Lateral = model->LayerInternal[i].Lateral;
  	  tempLayer->Power = model->LayerInternal[i].Power;
  	  tempLayer->Rx = 2.0 * getr(model->LayerInternal[i].K, height, width, l_chip, model->LayerInternal[i].Thickness);
  	  tempLayer->Ry = 2.0 * getr(model->LayerInternal[i].K, width, height, w_chip, model->LayerInternal[i].Thickness);
  	  tempLayer->Rz = model->LayerInternal[i].Rho * model->LayerInternal[i].Thickness / grid_area;
			tempLayer->flp_name = model->LayerInternal[i].flp_name;
			
		  /* default configs are overrided by the first layer in the layer config file */
		  if (i==0) {
  	  	model->t_chip = model->LayerInternal[i].Thickness;
  	  }
  	
			model->Layers[lvl].push_back(*tempLayer);
			delete tempLayer; 	
		}
	}
	
	return NumOfLayer;
}

/* Sets up the RC difference equations for the Grid model */
int32_t layer_C_setup(grid_model_t *model) 
{
	/*	shortcuts	*/
	double c_convec = model->config.c_convec;
	double s_sink = model->config.s_sink;
	double t_sink = model->config.t_sink;
	double s_spreader = model->config.s_spreader;
	double t_spreader = model->config.t_spreader;
	int32_t row = model->row;
	int32_t col = model->col;
		
	/* NOTE: *_mid - the vertical R/C from CENTER nodes of spreader 
	 * and heatsink. *_per - the vertical R/C from PERIPHERAL (n,s,e,w) nodes
	 */
	double c_sp_per, c_hs_mid, c_hs_per;
	double FittingFactor = 0.0;
	
	/*	get chip width and height from default floorplan */
	double w_chip = get_total_width(model->flp); /* x-axis */
	double l_chip = get_total_height(model->flp);	/* y-axis */
	
	/* sanity check on floorplan sizes	*/
	if (w_chip > s_sink || l_chip > s_sink || 
		w_chip > s_spreader || l_chip > s_spreader) {
		print_flp(model->flp);
		print_flp_fig(model->flp);
		fatal("inordinate floorplan size!\n");
	}	

	double height; 		/* dimensions of the grids when no multigrid algorithm */
	double width;				
	double grid_area;
	
	/*	C's of spreader and sink */
	c_sp_per = C_FACTOR * SPEC_HEAT_CU * t_spreader * (s_spreader * s_spreader - w_chip*l_chip) / 4.0;
	c_hs_mid = C_FACTOR * SPEC_HEAT_CU * t_sink * (s_spreader * s_spreader);
	c_hs_per = C_FACTOR * SPEC_HEAT_CU * t_sink * (s_sink * s_sink - s_spreader*s_spreader) / 4.0;

	model->Csp = c_sp_per;
	model->Csi = c_hs_per;
	model->Csic = c_hs_mid;	
	model->Cconv = c_convec;
	
	for (int32_t lvl =0; lvl <= model->mv_level; lvl++) {
		height = l_chip / (row * (lvl+1));	/* grid size for current multigrid level */
		width = w_chip / (col * (lvl+1));  
		grid_area = height * width;
	
		//Calculate the properties of each layer and store them in Layers
		for (int32_t i = 0; i < (int)model->LayerInternal.size(); i++) {
  		if (i != (int) (model->LayerInternal.size() - 1) ) {   //All layers except spreader layer
  	  	FittingFactor = C_FACTOR * ((model->LayerInternal[i].SpHtCap / model->LayerInternal[i+1].SpHtCap) *
  	                                          (w_chip + 0.88 * model->LayerInternal[i+1].Thickness) *
  	                                          (l_chip + 0.88 * model->LayerInternal[i+1].Thickness) *
  	                                          model->LayerInternal[i+1].Thickness /
  	                                          (w_chip * l_chip * model->LayerInternal[i].Thickness) + 1);
  	    }
  	    else {  //Spreader Layer
  	    	FittingFactor = C_FACTOR;
  	   	}
  	     	
			model->Layers[lvl][i].Cap = FittingFactor * model->LayerInternal[i].SpHtCap * model->LayerInternal[i].Thickness * grid_area;
			model->Layers[lvl][i].flp_name = model->LayerInternal[i].flp_name;

		  /* default configs are overrided by the first layer in the layer config file */			
		  if (i==0) {
  	  	model->factor_chip = FittingFactor;
  	  }  	
		}
	}
	
	return 1;
}

/* maximum power density possible (say 150W for a 10mm x 10mm chip)	*/
#define MAX_PD	(1.5e6)
/* required precision in degrees	*/
#define PRECISION	0.001
#define TOO_LONG	100000
#define MIN_ITER	1

// Calculates the transient temperature in the grid model
// simply convert the heat transfer differetial equations 
// to difference equations and solve the difference equations by iterating 
void compute_tran_temp(double time_elapsed, grid_model_t *model, vector<double> &temp, vector<double> &power) 
{
	int32_t i;
	/* shortcut	*/
	int32_t row = model->row;
	int32_t col = model->col;
	
	int32_t g_n = row * col;	

	int32_t OffSP = (model->Layers[0].size() - 1) * g_n;
	int32_t OffSI = model->Layers[0].size() * g_n;
	int32_t Spr = model->Layers[0].size() - 1;

	/* Number of Iteration Calculation */

	double delta_t;
	double h, no_of_iter;	
	double max_slope = MAX_PD / (model->factor_chip * model->t_chip * SPEC_HEAT_SI);
	
	h = PRECISION / max_slope;
	no_of_iter = time_elapsed / h;
	no_of_iter = (int) (no_of_iter > MIN_ITER) ? no_of_iter : MIN_ITER;	
	//	#if VERBOSE > 2
	cout << "Iterations: " << no_of_iter << endl;
	//	#endif
	delta_t = time_elapsed / no_of_iter;
	
	// dT for heat spreader peripheral nodes and heat sink nodes
	vector<double> delta_temp_extra(EXTRA);
	
	int32_t n;
	for (n = 0; n < no_of_iter; n++) {
		LayerTemperature (temp, power, model, delta_t);

		delta_temp_extra[0] = (delta_t / model->Csp) * (
						(SumTemp(temp, row, col, 0, 0, row-1, 0, OffSP, OffSI, 0))/model->Rsplex + 
						(temp[5+OffSI]-temp[OffSI])/model->Rspl + (temp[4+OffSI]-temp[OffSI])/model->Rspv);
		delta_temp_extra[1] = (delta_t / model->Csp) * ( 
						(SumTemp(temp, row, col, 0, col-1, row-1, col-1, OffSP, OffSI, 1))/model->Rsplex + 
						(temp[6+OffSI]-temp[1+OffSI])/model->Rspl + (temp[4+OffSI]-temp[1+OffSI])/model->Rspv);
		
		delta_temp_extra[2] = (delta_t / model->Csp) * (
						(SumTemp(temp, row, col, 0, 0, 0, col-1, OffSP, OffSI, 2))/model->Rspley + 
						(temp[7+OffSI]-temp[2+OffSI])/model->Rspl + (temp[4+OffSI]-temp[2+OffSI])/model->Rspv);

		delta_temp_extra[3] = (delta_t / model->Csp) * ( 
						(SumTemp(temp, row, col, row-1, 0, row-1, col-1, OffSP, OffSI, 3))/model->Rspley + 
						(temp[8+OffSI]-temp[3+OffSI])/model->Rspl + (temp[4+OffSI]-temp[3+OffSI])/model->Rspv);
		delta_temp_extra[4] = (delta_t / model->Csic) * (
						(SumTemp(temp, row, col, 0, 0, row-1, col-1, OffSP, OffSI, 4))/model->Layers[0][Spr].Rz + 
						(temp[OffSI] + temp[1+OffSI] + temp[2+OffSI] + temp[3+OffSI]-4.0*temp[4+OffSI])/model->Rspv + 
						(temp[9+OffSI]-temp[4+OffSI])/model->Rsicv);

		delta_temp_extra[5] = (delta_t / model->Csi) * ((temp[OffSI]-temp[5+OffSI])/model->Rspl + (temp[9+OffSI]-temp[5+OffSI])/model->Rsi);
		
		delta_temp_extra[6] = (delta_t / model->Csi) * ((temp[1+OffSI]-temp[6+OffSI])/model->Rspl + (temp[9+OffSI]-temp[6+OffSI])/model->Rsi);
	
		delta_temp_extra[7] = (delta_t / model->Csi) * ((temp[2+OffSI]-temp[7+OffSI])/model->Rspl + (temp[9+OffSI]-temp[7+OffSI])/model->Rsi);

		delta_temp_extra[8] = (delta_t / model->Csi) * ((temp[3+OffSI]-temp[8+OffSI])/model->Rspl + (temp[9+OffSI]-temp[8+OffSI])/model->Rsi);
		
		delta_temp_extra[9] = (delta_t / model->Cconv) * (
						(temp[5+OffSI] + temp[6+OffSI] + temp[7+OffSI] + temp[8+OffSI] - 4.0*temp[9+OffSI])/model->Rsi + 
						(temp[4+OffSI]-temp[9+OffSI])/model->Rsicv + (model->Tamb-temp[9+OffSI])/model->Rconv);

		//Update extra node temperatures
		for (i = 0; i < EXTRA; i++) {
			temp[i+OffSI] = temp[i+OffSI] + delta_temp_extra[i];
		}
	}
}

double SumTemp (vector<double> &temp, int32_t row, int32_t col, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t Offset, int32_t Offset2, int32_t Offset3) 
{
	int32_t i , j;
	double result = 0.0;
	if ((x2 > x1) && (y2 > y1)) {
		for (i = x1; i <= x2; i++) {
			for (j = y1; j <= y2; j++) {
				result = result + temp[i*col + j + Offset] - temp[Offset2+Offset3];
			}
		}
	}

	else if ((x2 == x1) && (y2 > y1)) {
		for (i = y1; i <= y2; i++) {
			result = result + temp[x1*col + i + Offset] - temp[Offset2+Offset3];
		}
	}

	else if ((x2 > x1) && (y2 == y1)) {
		for (i = x1; i <= x2; i++) {
			result = result + temp[i*col + y1 + Offset] - temp[Offset2+Offset3];
		}
	}
	return result;
}

// iteratively solve the difference equations for the internal nodes
void LayerTemperature (vector<double> &temp, vector<double> &power, grid_model_t *model, double delta_t) 
{
	/* shortcut	*/
	int32_t row = model->row;
	int32_t col = model->col;

	int32_t g_n =row * col;
	int32_t nL = model->Layers[0].size();		//Number Of Layers
	int32_t OffSet;
	int32_t NextOffSet;
	int32_t PrevOffSet;
	int32_t OffSI = nL * g_n;
	int32_t OffSP = (nL-1) * g_n;
	int32_t i = 0;

	for (int32_t r = 0; r < row; r++) {
	for (int32_t c = 0; c < col; c++) {

	if ( (r == 0) && (c == 0) ) {	//Corner 1
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[0 + OffSet] +
						(temp[1 + OffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rx +
						(temp[col + OffSet] - temp[0 + OffSet]) / model->Layers[0][i].Ry +
						(temp[0 + NextOffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[0 + OffSet] +
						(temp[0 + NextOffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[0 + OffSet] +
						(temp[1 + OffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rx +
						(temp[col + OffSet] - temp[0 + OffSet]) / model->Layers[0][i].Ry +
						(temp[0 + NextOffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rz +
						(temp[0 + PrevOffSet] - temp[0 + OffSet]) / model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[0 + OffSet] +
						(temp[0 + NextOffSet] - temp[0 + OffSet]) / model->Layers[0][i].Rz +
						(temp[0 + PrevOffSet] - temp[0 + OffSet]) / model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[0+OffSP] +  
			(temp[1+OffSP]-temp[OffSP])/model->Layers[0][i].Rx + (temp[col+OffSP]-temp[OffSP])/model->Layers[0][i].Ry + 
			(temp[OffSI+SP_W]-temp[OffSP])/model->Rsplex + (temp[OffSI+SP_N]-temp[OffSP])/model->Rspley +
			(temp[SP_B+OffSI]-temp[OffSP])/model->Layers[0][i].Rz + (temp[PrevOffSet]-temp[OffSP])/model->Layers[0][i-1].Rz);
	}

	else if ((r == 0) && (c == col-1)) {	//Corner 2
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c + OffSet] +
						(temp[c-1 + OffSet] - temp[c + OffSet]) / model->Layers[0][i].Rx +
						(temp[c+col + OffSet] - temp[c + OffSet]) / model->Layers[0][i].Ry +
						(temp[c + NextOffSet] - temp[c + OffSet]) / model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c + OffSet] +
						(temp[c + NextOffSet] - temp[c + OffSet]) / model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c + OffSet] +
						(temp[c-1 + OffSet] - temp[c + OffSet]) / model->Layers[0][i].Rx +
						(temp[c+col + OffSet] - temp[c + OffSet]) / model->Layers[0][i].Ry +
						(temp[c + NextOffSet] - temp[c + OffSet]) / model->Layers[0][i].Rz +
						(temp[c + PrevOffSet] - temp[c + OffSet]) / model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c + OffSet] +
						(temp[c + NextOffSet] - temp[c + OffSet]) / model->Layers[0][i].Rz +
						(temp[c + PrevOffSet] - temp[c + OffSet]) / model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c-1+OffSP] +  
			(temp[c-1+OffSP]-temp[c+OffSP])/model->Layers[0][i].Rx + 
			(temp[col+c+OffSP]-temp[c+OffSP])/model->Layers[0][i].Ry + 
			(temp[SP_N+OffSI]-temp[c+OffSP])/model->Rspley + (temp[SP_E+OffSI]-temp[c+OffSP])/model->Rsplex +
			(temp[SP_B+OffSI]-temp[c+OffSP])/model->Layers[0][i].Rz + 
			(temp[c+PrevOffSet]-temp[c+OffSP])/model->Layers[0][i-1].Rz);
	}

	else if ((r == row - 1) && (c == col - 1)) {	//Corner 3
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c + OffSet] + 
						(temp[r*col+c-1+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+c+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c + OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c + OffSet] + 
						(temp[r*col+c-1+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+c+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c + OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
			}

		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSP] + 
			(temp[r*col+c-1+OffSP]-temp[r*col+c+OffSP])/model->Layers[0][i].Rx + 
			(temp[(r-1)*col+c+OffSP]-temp[r*col+c+OffSP])/model->Layers[0][i].Ry + 
			(temp[SP_S+OffSI]-temp[r*col+c+OffSP])/model->Rspley + (temp[SP_E+OffSI]-temp[r*col+c+OffSP])/model->Rsplex +
			(temp[SP_B+OffSI]-temp[r*col+c+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSP])/model->Layers[0][i-1].Rz);
					
	}
	
	else if ((r == row - 1) && (c == 0)) {	//Corner 4
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+1+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+1+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz + 
						(temp[r*col+PrevOffSet]-temp[r*col+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz + 
						(temp[r*col+PrevOffSet]-temp[r*col+OffSet])/model->Layers[0][i-1].Rz);
				}
			}

		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSP] + 
			(temp[r*col+1+OffSP]-temp[r*col+OffSP])/model->Layers[0][i].Rx + 
			(temp[(r-1)*col+OffSP]-temp[r*col+OffSP])/model->Layers[0][i].Ry + 
			(temp[SP_S+OffSI]-temp[r*col+OffSP])/model->Rspley + (temp[SP_W+OffSI]-temp[r*col+OffSP])/model->Rsplex +
			(temp[SP_B+OffSI]-temp[r*col+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+PrevOffSet]-temp[r*col+OffSP])/model->Layers[0][i-1].Rz);
					
	}

	else if (r == 0) {	//Edge 1
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c+OffSet] + 
						(temp[c+1+OffSet] + temp[c-1+OffSet]-2.0*temp[c+OffSet])/model->Layers[0][i].Rx + 
						(temp[col+c+OffSet]-temp[c+OffSet])/model->Layers[0][i].Ry + 
						(temp[c+NextOffSet]-temp[c+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c+OffSet] + 
						(temp[c+NextOffSet]-temp[c+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c+OffSet] + 
						(temp[c+1+OffSet] + temp[c-1+OffSet]-2.0*temp[c+OffSet])/model->Layers[0][i].Rx + 
						(temp[col+c+OffSet]-temp[c+OffSet])/model->Layers[0][i].Ry + 
						(temp[c+NextOffSet]-temp[c+OffSet])/model->Layers[0][i].Rz +
						(temp[c+PrevOffSet]-temp[c+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c+OffSet] + 
						(temp[c+NextOffSet]-temp[c+OffSet])/model->Layers[0][i].Rz +
						(temp[c+PrevOffSet]-temp[c+OffSet])/model->Layers[0][i-1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[c+OffSP] + 
			(temp[c+1+OffSP]+temp[c-1+OffSP]-2.0*temp[c+OffSP])/model->Layers[0][i].Rx + 
			(temp[col+c+OffSP]-temp[c+OffSP])/model->Layers[0][i].Ry + 
			(temp[SP_N+OffSI]-temp[c+OffSP])/model->Rspley + (temp[SP_B+OffSI]-temp[c+OffSP])/model->Layers[0][i].Rz + 
			(temp[c+PrevOffSet]-temp[c+OffSP])/model->Layers[0][i-1].Rz);
						
	}

	else if (c == col - 1) {	//Edge 2
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[(r+1)*col+c+OffSet] + temp[(r-1)*col+c+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c-1+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[(r+1)*col+c+OffSet] + temp[(r-1)*col+c+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c-1+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSP] + 
			(temp[(r-1)*col+c+OffSP]+temp[(r+1)*col+c+OffSP]-2.0*temp[r*col+c+OffSP])/model->Layers[0][i].Ry + 
			(temp[r*col+c-1+OffSP]-temp[r*col+c+OffSP])/model->Layers[0][i].Rx + 
			(temp[SP_E+OffSI]-temp[r*col+c+OffSP])/model->Rsplex + 
			(temp[SP_B+OffSI]-temp[r*col+c+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSP])/model->Layers[0][i-1].Rz);
						
	}
		
	else if (r == row - 1) {	//Edge 3
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+1+OffSet]+temp[r*col+c-1+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+c+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+1+OffSet]+temp[r*col+c-1+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[(r-1)*col+c+OffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSP] + 
			(temp[r*col+c+1+OffSP]+temp[r*col+c-1+OffSP]-2.0*temp[r*col+c+OffSP])/model->Layers[0][i].Rx + 
			(temp[(r-1)*col+c+OffSP]-temp[r*col+c+OffSP])/model->Layers[0][i].Ry + 
			(temp[SP_S+OffSI]-temp[r*col+c+OffSP])/model->Rspley + 
			(temp[SP_B+OffSI]-temp[r*col+c+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSP])/model->Layers[0][i-1].Rz);
						
	}

	else if (c == 0) {	//Edge 4
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[(r+1)*col+OffSet]+temp[(r-1)*col+OffSet]-2.0*temp[r*col+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+1+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[(r+1)*col+OffSet]+temp[(r-1)*col+OffSet]-2.0*temp[r*col+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+1+OffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+PrevOffSet]-temp[r*col+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSet] + 
						(temp[r*col+NextOffSet]-temp[r*col+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+PrevOffSet]-temp[r*col+OffSet])/model->Layers[0][i-1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+OffSP] + 
			(temp[(r+1)*col+OffSP]+temp[(r-1)*col+OffSP]-2.0*temp[r*col+OffSP])/model->Layers[0][i].Ry + 
			(temp[r*col+1+OffSP]-temp[r*col+OffSP])/model->Layers[0][i].Rx + 
			(temp[SP_W+OffSI]-temp[r*col+OffSP])/model->Rsplex + 
			(temp[SP_B+OffSI]-temp[r*col+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+PrevOffSet]-temp[r*col+OffSP])/model->Layers[0][i-1].Rz);
						
	}

	else {	//Inside the chip
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[(r+1)*col+c+OffSet]+temp[(r-1)*col+c+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+1+OffSet]+temp[r*col+c-1+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[(r+1)*col+c+OffSet]+temp[(r-1)*col+c+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Ry + 
						(temp[r*col+c+1+OffSet]+temp[r*col+c-1+OffSet]-2.0*temp[r*col+c+OffSet])/model->Layers[0][i].Rx + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSet] + 
						(temp[r*col+c+NextOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i].Rz +
						(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSet])/model->Layers[0][i-1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		model->Layers[0][i].Delta_temp = (delta_t / model->Layers[0][i].Cap) * (power[r*col+c+OffSP] + 
			(temp[(r+1)*col+c+OffSP]+temp[(r-1)*col+c+OffSP]-2.0*temp[r*col+c+OffSP])/model->Layers[0][i].Ry + 
			(temp[r*col+c+1+OffSP]+temp[r*col+c-1+OffSP]-2.0*temp[r*col+c+OffSP])/model->Layers[0][i].Rx + 
			(temp[SP_B+OffSI]-temp[r*col+c+OffSP])/model->Layers[0][i].Rz + 
			(temp[r*col+c+PrevOffSet]-temp[r*col+c+OffSP])/model->Layers[0][i-1].Rz);
						
	}

	//Update Temperatures
	for (i = 0; i < (int) model->Layers[0].size(); i++) {
		OffSet = i * g_n;
		temp[r*col+c+OffSet] = temp[r*col+c+OffSet] + model->Layers[0][i].Delta_temp;
	}
	
}
}
}

int32_t convergence_check(vector<double> &steady_temp, vector <double> &prev)
{
	uint32_t i;
	for(i=0; i < steady_temp.size(); i++)
		if (!eq(steady_temp[i], prev[i]))
			return FALSE;
	return TRUE;		
}

//Calculate the steady temperature
void compute_steady_temp(grid_model_t *model, vector<double> &steady_temp, vector<double> &power)
{
	/* shortcut	*/
	int32_t row = model->row;
	int32_t col = model->col;

	int32_t g_n = row * col;

	int32_t OffSP = (model->Layers[0].size() - 1) * g_n;
	int32_t OffSI = model->Layers[0].size() * g_n;
	int32_t Spr = model->Layers[0].size() - 1;
	
	/* Number of Iteration Calculation */
	int32_t no_of_iter	= 500; // minimum number of iterations
	
	/* empirically determine number of iterations for the steady-state solver 					*/
	/* only valid for normal die size: about 20mm x 20mm 																*/
	/* if using significantly different die size, adjust no_of_iter accordingly 				*/
	/* larger die size--less iterations, because grid size is also larger								*/
	/* smaller die size--more iterations, because grid size is also smaller							*/
	
	if ((row>32 || col>32) && (row<128 && col <128)) {
		no_of_iter = (row >= col) ? (500*((int)pow(row/32.0,2.0))) : (500*((int)pow(col/32.0,2.0)));
		if (no_of_iter > 5000) {
			no_of_iter = 5000;
		}
	}
	
	else if ((row>=128 || col>=128) && (row<1024 && col <1024)) {
		no_of_iter = (row >= col) ? (5000*((int)pow(row/128.0,2.0))) : (5000*((int)pow(col/128.0,2.0)));
		if (no_of_iter>=10000) {
			no_of_iter = 10000;
		}
	}
	
	else if (row>=1024 || col>=1024) 
		no_of_iter = (row >= col) ? (10000*((int)pow(row/1024.0,2.0))) : (10000*((int)pow(col/1024.0,2.0)));
	#if VERBOSE > 2	
	cout << "Number of iterations for steady-state solution is " << no_of_iter << endl;
	#endif
		
	if (no_of_iter>=10000) 
		cout << "May take a long time to finish..." << endl;
 
	int32_t n = 0;
	int32_t converged = FALSE;
//	for (n = 0; n < no_of_iter; n++) {
	vector<double> prev = steady_temp;
	while(!converged) {				
		LayerSteadyTemperature (steady_temp, power, model);
	
		steady_temp[0+OffSI] = (SteadySumTemp(steady_temp, row, col, 0, 0, row-1, 0, OffSP, OffSI, 0)/model->Rsplex + 
														steady_temp[5+OffSI]/model->Rspl + steady_temp[4+OffSI]/model->Rspv) / (row/model->Rsplex+1.0/model->Rspl+1.0/model->Rspv);
														
		steady_temp[1+OffSI] = (SteadySumTemp(steady_temp, row, col, 0, col-1, row-1, col-1, OffSP, OffSI, 1)/model->Rsplex + 
														steady_temp[6+OffSI]/model->Rspl + steady_temp[4+OffSI]/model->Rspv) / (row/model->Rsplex+1.0/model->Rspl+1.0/model->Rspv);
														
		steady_temp[2+OffSI] = (SteadySumTemp(steady_temp, row, col, 0, 0, 0, col-1, OffSP, OffSI, 2)/model->Rspley + 
														steady_temp[7+OffSI]/model->Rspl + steady_temp[4+OffSI]/model->Rspv) / (col/model->Rspley+1.0/model->Rspl+1.0/model->Rspv);
														
		steady_temp[3+OffSI] = (SteadySumTemp(steady_temp, row, col, row-1, 0, row-1, col-1, OffSP, OffSI, 3)/model->Rspley + 
														steady_temp[8+OffSI]/model->Rspl + steady_temp[4+OffSI]/model->Rspv) / (col/model->Rspley+1.0/model->Rspl+1.0/model->Rspv);
														
		steady_temp[4+OffSI] = (SteadySumTemp(steady_temp, row, col, 0, 0, row-1, col-1, OffSP, OffSI, 4)/model->Layers[0][Spr].Rz + 
														steady_temp[9+OffSI]/model->Rsicv + (steady_temp[OffSI] + steady_temp[1+OffSI] + 
														steady_temp[2+OffSI] + steady_temp[3+OffSI])/model->Rspv) / (row*col/model->Layers[0][Spr].Rz+1.0/model->Rsicv+4.0/model->Rspv);
																										
		steady_temp[5+OffSI] = (steady_temp[0+OffSI]/model->Rspl + steady_temp[9+OffSI]/model->Rsi)/(1.0/model->Rspl+1.0/model->Rsi);	
			
		steady_temp[6+OffSI] = (steady_temp[1+OffSI]/model->Rspl + steady_temp[9+OffSI]/model->Rsi)/(1.0/model->Rspl+1.0/model->Rsi);
		
		steady_temp[7+OffSI] = (steady_temp[2+OffSI]/model->Rspl + steady_temp[9+OffSI]/model->Rsi)/(1.0/model->Rspl+1.0/model->Rsi);
		
		steady_temp[8+OffSI] = (steady_temp[3+OffSI]/model->Rspl + steady_temp[9+OffSI]/model->Rsi)/(1.0/model->Rspl+1.0/model->Rsi);
		
		steady_temp[9+OffSI] = ((steady_temp[5+OffSI]+steady_temp[6+OffSI]+steady_temp[7+OffSI]+steady_temp[8+OffSI])/model->Rsi+
														steady_temp[4+OffSI]/model->Rsicv+model->Tamb/model->Rconv)/(4.0/model->Rsi+1.0/model->Rsicv+1.0/model->Rconv);
		converged = convergence_check(steady_temp, prev);
		prev = steady_temp;
		n++;
	}
	#if VERBOSE > 2	
	fprintf(stdout, "iterations to converge: %d\n", n);
	#endif
}

double SteadySumTemp (vector<double> &steady_temp, int32_t row, int32_t col, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t Offset, int32_t Offset2, int32_t Offset3)
{
	int32_t i , j;
	double result = 0.0;
	if ((x2 > x1) && (y2 > y1)) {
		for (i = x1; i <= x2; i++) {
			for (j = y1; j <= y2; j++) {
				result = result + steady_temp[i*col + j + Offset];
			}
		}
	}

	else if ((x2 == x1) && (y2 > y1)) {
		for (i = y1; i <= y2; i++) {
			result = result + steady_temp[x1*col + i + Offset];
		}
	}

	else if ((x2 > x1) && (y2 == y1)) {
		for (i = x1; i <= x2; i++) {
			result = result + steady_temp[i*col + y1 + Offset];
		}
	}
	return result;
}


void LayerSteadyTemperature (vector<double> &steady_temp, vector<double> &power, grid_model_t *model) 
{
	/* shortcut	*/
	int32_t row = model->row;
	int32_t col = model->col;

	int32_t g_n =row * col;
	int32_t nL = model->Layers[0].size();		//Number Of Layers
	int32_t OffSet;
	int32_t NextOffSet;
	int32_t PrevOffSet;
	int32_t OffSI = nL * g_n;
	int32_t OffSP = (nL-1) * g_n;
	int32_t i = 0;
	
for (int32_t r = 0; r < row; r++) {
	for (int32_t c = 0; c < col; c++) {

	if ( (r == 0) && (c == 0) ) {	//Corner 1
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[0+OffSet] = (power[0+OffSet]+steady_temp[1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[0+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[0 + OffSet] = (power[0 + OffSet]+steady_temp[0+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[0 + OffSet] = (power[0 + OffSet]+steady_temp[1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[0+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[0+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[0 + OffSet] = (power[0 + OffSet]+steady_temp[0+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[0 + PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[OffSP]=(steady_temp[1+OffSP]/model->Layers[0][i].Rx+steady_temp[col+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_W]/model->Rsplex+steady_temp[OffSI+SP_N]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rsplex+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}

	else if ((r == 0) && (c == col-1)) {	//Corner 2
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[c + OffSet] = (power[c + OffSet]+steady_temp[c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[c+col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[c + OffSet] = (power[c + OffSet]+steady_temp[c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[c + OffSet] = (power[c + OffSet]+steady_temp[c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[c+col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[c + OffSet] = (power[c + OffSet]+steady_temp[c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[c + PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[c+OffSP]=(steady_temp[c-1+OffSP]/model->Layers[0][i].Rx+steady_temp[c+col+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_E]/model->Rsplex+steady_temp[OffSI+SP_N]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[c+PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rsplex+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}

	else if ((r == row - 1) && (c == col - 1)) {	//Corner 3
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c + OffSet]+steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c-col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);					
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+c + OffSet] = (power[r*col+c + OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+c + OffSet] = (power[r*col+c + OffSet]+steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c-col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+c + OffSet] = (power[r*col+c + OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c + PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}

		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+c+OffSP]=( steady_temp[r*col+c-1+OffSP]/model->Layers[0][i].Rx+steady_temp[(r-1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_E]/model->Rsplex+steady_temp[OffSI+SP_S]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rsplex+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}
	
	else if ((r == row - 1) && (c == 0)) {	//Corner 4
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+OffSet] = (power[r*col + OffSet]+steady_temp[r*col+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[(r-1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);		
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[r*col+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[(r-1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}

		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+OffSP]=(steady_temp[r*col+1+OffSP]/model->Layers[0][i].Rx+steady_temp[(r-1)*col+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_W]/model->Rsplex+steady_temp[OffSI+SP_S]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rsplex+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}

	else if (r == 0) {	//Edge 1
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[c+OffSet] = (power[c+OffSet]+steady_temp[c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[c-1+OffSet]/model->Layers[0][i].Rx+steady_temp[col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[c+NextOffSet]/model->Layers[0][i].Rz)/(2.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);		
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[c+OffSet] = (power[c+OffSet]+steady_temp[c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[c+OffSet] = (power[c+OffSet]+steady_temp[c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(2.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[c+OffSet] = (power[c+OffSet]+steady_temp[c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[c+OffSP]=(steady_temp[c+1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[c-1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_N]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[c+PrevOffSet]/model->Layers[0][i-1].Rz)/(2.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}

	else if (c == col - 1) {	//Edge 2
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[(r+1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rx+
																		2.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);		
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[(r+1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		2.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+c+OffSP]=(steady_temp[(r+1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[(r-1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[r*col+c-1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[OffSI+SP_E]/model->Rsplex+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+2.0/model->Layers[0][i].Ry+1.0/model->Rsplex+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}
		
	else if (r == row - 1) {	//Edge 3
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(2.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz);		
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(2.0/model->Layers[0][i].Rx+
																		1.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+c+OffSP]=(steady_temp[r*col+c+1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[r*col+c-1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[(r-1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[OffSI+SP_S]/model->Rspley+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i-1].Rz)/(2.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Ry+1.0/model->Rspley+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}

	else if (c == 0) {	//Edge 4
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[(r+1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz)/(2.0/model->Layers[0][i].Ry+
																		1.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Rz);		
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[(r+1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rx+
																		2.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+OffSet] = (power[r*col+OffSet]+steady_temp[r*col+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}
		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+OffSP]=(steady_temp[(r+1)*col+OffSP]/model->Layers[0][i].Ry+
												steady_temp[(r-1)*col+OffSP]/model->Layers[0][i].Ry+
												steady_temp[r*col+1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[OffSI+SP_W]/model->Rsplex+steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+PrevOffSet]/model->Layers[0][i-1].Rz)/(1.0/model->Layers[0][i].Rx+2.0/model->Layers[0][i].Ry+1.0/model->Rsplex+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
}
	else {	//Inside the chip
		for (i = 0; i < (int) model->Layers[0].size() - 1; i++) {
			OffSet = model->Layers[0][i].Id * g_n;
			NextOffSet = model->Layers[0][i + 1].Id * g_n;
			if (i == 0) {
				if (model->Layers[0][i].Lateral) {	//Top Layer AND Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[(r+1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(2.0/model->Layers[0][i].Ry+
																		2.0/model->Layers[0][i].Rx+1.0/model->Layers[0][i].Rz);	
				}
				else	//Top Layer BUT No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz)/(1.0/model->Layers[0][i].Rz);
				}
			}
			else {
				PrevOffSet = model->Layers[0][i - 1].Id * g_n;
				if (model->Layers[0][i].Lateral) {	//Not Top Layer, Lateral Heat Flow
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[(r+1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[(r-1)*col+c+OffSet]/model->Layers[0][i].Ry+
																		steady_temp[r*col+c+1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c-1+OffSet]/model->Layers[0][i].Rx+
																		steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(2.0/model->Layers[0][i].Rx+
																		2.0/model->Layers[0][i].Ry+1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
				else	//Not Top Layer, No Lateral Heat Flow
				{
					steady_temp[r*col+c+OffSet] = (power[r*col+c+OffSet]+steady_temp[r*col+c+NextOffSet]/model->Layers[0][i].Rz+
																		steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i - 1].Rz)/(1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i - 1].Rz);
				}
			}
		}

		//Spreader
		i = nL-1;
		PrevOffSet = (i - 1) * g_n;
		steady_temp[r*col+c+OffSP]=(steady_temp[(r+1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[(r-1)*col+c+OffSP]/model->Layers[0][i].Ry+
												steady_temp[r*col+c+1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[r*col+c-1+OffSP]/model->Layers[0][i].Rx+
												steady_temp[SP_B+OffSI]/model->Layers[0][i].Rz+
												steady_temp[r*col+c+PrevOffSet]/model->Layers[0][i-1].Rz)/(2.0/model->Layers[0][i].Rx+2.0/model->Layers[0][i].Ry+
												1.0/model->Layers[0][i].Rz+1.0/model->Layers[0][i-1].Rz);
	}
	}
 }
}

/* set the size and inital values for the power of each grid cell*/
void init_gpower(int32_t layer_total, grid_model_t *model)
{
	int32_t g_row = model->row;
	int32_t g_col = model->col;
	model->g_power.resize(layer_total*g_row*g_col+EXTRA, 0.0);
}

/* Initialize block and grid structures from floorplan files for each layer*/
int32_t grid_setup (grid_model_t *model, double init_temp) 
{	
	int32_t nL = model->Layers[0].size();
	int32_t total_block = 0;
	
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	model->g_temp.resize(nL*g_row*g_col+EXTRA, init_temp);
	model->steady_g_temp.resize(nL*g_row*g_col+EXTRA, init_temp);
 
	double chip_width;
	double chip_height;
	double x1, y1, x2, y2, pow;

	flp_t *flp; 
	FBlocks *B;
	Grids *G;
	FloorplanDetails *temp;

  int32_t row = g_row;
	int32_t column = g_col;
	int32_t k = 0;
  int32_t i = 0;
  
	model->sum_n_units = 0;	

	for (k = 0; k < nL; k++) {
		flp = read_flp(const_cast<char *>(model->Layers[0][k].flp_name.c_str()), FALSE);
		
 		chip_width = get_total_width(flp);
		chip_height = get_total_height(flp);
		
		/* all layers in the layer config file should have the same width and length */
		if (!eq(chip_width, model->chip_width) || !eq(chip_height, model->chip_height))
			fatal("layer width and length are not the same as the default floorplan!\n");
		
		/* initiate functional block from floorplan */
		temp = new FloorplanDetails;
		i = 0;
		for (i = 0; i < flp->n_units; i++) {
			x1 = flp->units[i].leftx;	/* (x1,y1) points to the upper-left corner, different from floorplan file. */
			y1 = chip_height - (flp->units[i].bottomy + flp->units[i].height);
			x2 = x1 + flp->units[i].width;
			y2 = y1 + flp->units[i].height;
			pow = 0.0;
			
			B = new FBlocks(x1, y1, x2, y2, pow, i);
			temp->FBlockChip.push_back (*B);
			delete B;
		}

		/* initiate grids */
		for (i = 0; i <= (row * column); i++) {
			//Dummy grid element. Grid element index starts from 1!
			if (i == 0) {
				G = new Grids (row, column, 1, chip_width, chip_height);
				temp->GridChip.push_back (*G);
			}
			else {
				G = new Grids (row, column, i, chip_width, chip_height);
				temp->GridChip.push_back (*G);
			}
			delete G;
		}
		
		//Call updateGridcoord to setup grid coordinates for the blocks
		for (i = 0; i < (int) temp->FBlockChip.size(); i++) {
			temp->FBlockChip[i].updateGridcoord(row, column, chip_width, chip_height);
		}

		//Call PartPower to determine the keep area-weighted information for each block and grid
		for (i = 0; i < (int) temp->FBlockChip.size(); i++) {
			temp->FBlockChip[i].PartPower(g_col, temp->GridChip);
		}
	
		for (i = 0; i < (int) temp->FBlockChip.size(); i++) {
			temp->FBlockChip[i].SetGridUpdate(true);
		}

		/* number of blocks including those in TIM and spreader layers*/		
			total_block += flp->n_units;
			
		if (model->Layers[0][k].Power)  // only count functional units in layers dissipating power
			model->sum_n_units += flp->n_units;

		model->Floorplan.push_back(*temp);
		delete temp;
		free_flp(flp, 0);
	} //end of set up grids

	return total_block;
}

/* convert block power to grid power */
void trans_bpow_gpow (vector<double> &power, grid_model_t *model) 
{
	int32_t i;
	/* shortcut	*/
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	int32_t g_n = g_row * g_col;
	int32_t nL = model->Layers[0].size();
 	int32_t j = 0;
	int32_t GridOffSet = 0;
	int32_t BlockOffSet = 0;

	for (j = 0; j < nL; j++) {
			GridOffSet = j * g_n;
			if (model->Layers[0][j].Power == true) {
				for (i = 1; i < (int) model->Floorplan[j].GridChip.size(); i++) {
					model->g_power[i-1+GridOffSet] = model->Floorplan[j].GridChip[i].CalculateGridPower(power, 
						BlockOffSet, model->Floorplan[j].FBlockChip); 
				}                                                                           
				BlockOffSet = BlockOffSet + model->Floorplan[j].FBlockChip.size();
			}
			else {
				for (i = 1; i < (int) model->Floorplan[j].GridChip.size(); i++) {
					model->g_power[i-1+GridOffSet] = 0.0;
				}
			}
	}
}

/* convert initial transient block temp to grid temp, not used in this release*/
void trans_tran_btemp_gtemp(vector<double> &temp, grid_model_t *model)
{
	int32_t i;
	/* shortcut	*/
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	int32_t g_n = g_row * g_col;
	int32_t GridOffSet = 0;
	int32_t BlockOffSet = 0;

	int32_t j;
	for (j = 0; j < (int) model->Layers[0].size(); j++) {
		GridOffSet = j * g_n;
		for (i = 1; i < (int) model->Floorplan[j].GridChip.size(); i++) {
			model->g_temp[i-1+GridOffSet] = model->Floorplan[j].GridChip[i].CalculateGridTemp(temp, BlockOffSet);
		}
		BlockOffSet = BlockOffSet + model->Floorplan[j].FBlockChip.size();
	}

	//copy the extra nodes temperatures in the heatspreader and heatsink
	j = model->Layers[0].size();
	GridOffSet = j * g_n;
	for (i = 0; i < EXTRA; i++) {
		model->g_temp[i + GridOffSet] = temp[i + BlockOffSet];
	}
}

/* convert initial steady block temp to grid temp, not used in this release*/
void trans_steady_btemp_gtemp(vector<double> &temp, grid_model_t *model)
{
	int32_t i;
	/* shortcut	*/
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	int32_t g_n = g_row * g_col;
	int32_t GridOffSet = 0;
	int32_t BlockOffSet = 0;

	int32_t j;
	for (j = 0; j < (int) model->Layers[0].size(); j++) {
		GridOffSet = j * g_n;
		for (i = 1; i < (int) model->Floorplan[j].GridChip.size(); i++) {
			model->steady_g_temp[i-1+GridOffSet] = model->Floorplan[j].GridChip[i].CalculateGridTemp(temp, BlockOffSet);
		}
		BlockOffSet = BlockOffSet + model->Floorplan[j].FBlockChip.size();
	}

	//copy the extra nodes temperatures in the heatspreader and heatsink
	j = model->Layers[0].size();
	GridOffSet = j * g_n;
	for (i = 0; i < EXTRA; i++) {
		model->steady_g_temp[i + GridOffSet] = temp[i + BlockOffSet];
	}
}

/* convert transient grid temperatures to transient block temperatures*/
void trans_tran_gtemp_btemp(vector<double> &temp, grid_model_t *model)
{
	/* shortcut	*/
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	int32_t g_n = g_row * g_col;
	int32_t i, j;
	int32_t GridOffSet = 0;
	int32_t BlockOffSet = 0;

	for (j = 0; j < (int) model->Layers[0].size(); j++) {
			GridOffSet = j * g_n;
			int32_t n = model->Floorplan[j].FBlockChip.size();
			for (i = 0; i < n; i++) {
				temp[i+BlockOffSet] = model->Floorplan[j].FBlockChip[i].CalculateTemp(model->g_temp, 
									  GridOffSet, model->map_mode, g_col);
			}						  
			BlockOffSet = BlockOffSet + model->Floorplan[j].FBlockChip.size();
	}
	for (i=0; i<EXTRA; i++) {
		temp[model->total_n_blocks+i]=model->g_temp[i+ model->Layers[0].size()* g_n];
	}
}

/* convert steady grid temperatures to steady block temperatures*/
void trans_steady_gtemp_btemp(vector<double> &steady_temp, grid_model_t *model)
{
	/* shortcut	*/
	int32_t g_row = model->row;
	int32_t g_col = model->col;

	int32_t g_n = g_row * g_col;
	int32_t i, j;
	int32_t GridOffSet = 0;
	int32_t BlockOffSet = 0;

	for (j = 0; j < (int) model->Layers[0].size(); j++) {
			GridOffSet = j * g_n;
			//convert temperatures from grid to block
			int32_t n = model->Floorplan[j].FBlockChip.size();
			
			for (i = 0; i < n; i++) {
				steady_temp[i+BlockOffSet] = model->Floorplan[j].FBlockChip[i].CalculateTemp(model->steady_g_temp, 
											 GridOffSet, model->map_mode, g_col);
			}
			BlockOffSet = BlockOffSet + model->Floorplan[j].FBlockChip.size();
	}
	
	for (i=0; i<EXTRA; i++) {
		steady_temp[model->total_n_blocks+i]=model->steady_g_temp[i+ model->Layers[0].size()* g_n];
	}
}

/* dummy transient temperature solver in the grid model */
void fast_tran_solver(double sampling_intvl, grid_model_t *model)
{
		compute_tran_temp(sampling_intvl, model, model->g_temp, model->g_power);
}

/* dummy transient temperature solver in the grid model */
void fast_steady_solver(grid_model_t *model) 
{
		compute_steady_temp(model, model->steady_g_temp, model->g_power);
}
