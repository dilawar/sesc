#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "temperature.h"
#include "temperature_block.h"
#include "temperature_grid.h"
#include "flp.h"
#include "util.h"

/* default thermal configuration parameters	*/
thermal_config_t default_thermal_config(void)
{
	thermal_config_t config;

	/* chip specs	*/
	config.t_chip = 0.5e-3;				/* chip thickness in meters	*/
	/* temperature threshold for DTM (Kelvin)*/
	config.thermal_threshold = 111.8 + 273.15;

	/* heat sink specs	*/
	config.c_convec = 140.4;			/* convection capacitance in J/K */
	config.r_convec = 0.1;				/* convection resistance in K/W	*/
	config.s_sink = 60e-3;				/* heatsink side in m	*/
	config.t_sink = 6.9e-3; 			/* heatsink thickness  in m	*/

	/* heat spreader specs	*/
	config.s_spreader = 30e-3;			/* spreader side in m	*/
	config.t_spreader = 1e-3;			/* spreader thickness in m	*/

	/* interface material specs	*/
	config.t_interface = 75e-6;			/* interface material thickness in m */

	/* others	*/
	config.ambient = 40 + 273.15;		/* in kelvin	*/
	/* initial temperatures	from file	*/
	strcpy(config.init_file, NULLFILE);	
	config.init_temp = 60 + 273.15;		/* in Kelvin	*/
	/* steady state temperatures to file	*/
	strcpy(config.steady_file, NULLFILE);
 	/* 3.33 us sampling interval = 10K cycles at 3GHz	*/
	config.sampling_intvl = 3.333e-6;
	config.base_proc_freq = 3e9;		/* base processor frequency in Hz	*/
	config.dtm_used = FALSE;			/* set accordingly	*/
	/* set block model as default	*/
	strcpy(config.model_type, BLOCK_MODEL_STR);

	/* block model specific parameters	*/
	config.block_omit_lateral = FALSE;	/* omit lateral chip resistances?	*/

	/* grid model specific parameters	*/
	config.grid_rows = 50;				/* grid resolution - no. of rows	*/
	config.grid_cols = 50;				/* grid resolution - no. of cols	*/
	/* layer configuration from	file */
	strcpy(config.grid_layer_file, "layer.lcf");
	/* output steady state grid temperatures apart from block temperatures */
	strcpy(config.grid_steady_file, NULLFILE);
	/* 
	 * mapping mode between block and grid models.
	 * default: use the average temperature of the 
	 * grid cells as that of the entire block
	 */
	strcpy(config.grid_map_mode, GRID_AVG_STR);

	return config;
}

/* 
 * parse a table of name-value string pairs and add the configuration
 * parameters to 'config'
 */
void thermal_config_add_from_strs(thermal_config_t *config, str_pair *table, int32_t size)
{
	int32_t idx;
	if ((idx = get_str_index(table, size, "t_chip")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->t_chip) != 1)
			fatal("invalid format for configuration  parameter t_chip");
	if ((idx = get_str_index(table, size, "thermal_threshold")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->thermal_threshold) != 1)
			fatal("invalid format for configuration  parameter thermal_threshold");
	if ((idx = get_str_index(table, size, "c_convec")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->c_convec) != 1)
			fatal("invalid format for configuration  parameter c_convec");
	if ((idx = get_str_index(table, size, "r_convec")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->r_convec) != 1)
			fatal("invalid format for configuration  parameter r_convec");
	if ((idx = get_str_index(table, size, "s_sink")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->s_sink) != 1)
			fatal("invalid format for configuration  parameter s_sink");
	if ((idx = get_str_index(table, size, "t_sink")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->t_sink) != 1)
			fatal("invalid format for configuration  parameter t_sink");
	if ((idx = get_str_index(table, size, "s_spreader")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->s_spreader) != 1)
			fatal("invalid format for configuration  parameter s_spreader");
	if ((idx = get_str_index(table, size, "t_spreader")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->t_spreader) != 1)
			fatal("invalid format for configuration  parameter t_spreader");
	if ((idx = get_str_index(table, size, "t_interface")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->t_interface) != 1)
			fatal("invalid format for configuration  parameter t_interface");
	if ((idx = get_str_index(table, size, "ambient")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->ambient) != 1)
			fatal("invalid format for configuration  parameter ambient");
	if ((idx = get_str_index(table, size, "init_file")) >= 0)
		if(sscanf(table[idx].value, "%s", config->init_file) != 1)
			fatal("invalid format for configuration  parameter init_file");
	if ((idx = get_str_index(table, size, "init_temp")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->init_temp) != 1)
			fatal("invalid format for configuration  parameter init_temp");
	if ((idx = get_str_index(table, size, "steady_file")) >= 0)
		if(sscanf(table[idx].value, "%s", config->steady_file) != 1)
			fatal("invalid format for configuration  parameter steady_file");
	if ((idx = get_str_index(table, size, "sampling_intvl")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->sampling_intvl) != 1)
			fatal("invalid format for configuration  parameter sampling_intvl");
	if ((idx = get_str_index(table, size, "base_proc_freq")) >= 0)
		if(sscanf(table[idx].value, "%lf", &config->base_proc_freq) != 1)
			fatal("invalid format for configuration  parameter base_proc_freq");
	if ((idx = get_str_index(table, size, "dtm_used")) >= 0)
		if(sscanf(table[idx].value, "%d", &config->dtm_used) != 1)
			fatal("invalid format for configuration  parameter dtm_used");
	if ((idx = get_str_index(table, size, "model_type")) >= 0)
		if(sscanf(table[idx].value, "%s", config->model_type) != 1)
			fatal("invalid format for configuration  parameter model_type");
	if ((idx = get_str_index(table, size, "block_omit_lateral")) >= 0)
		if(sscanf(table[idx].value, "%d", &config->block_omit_lateral) != 1)
			fatal("invalid format for configuration  parameter block_omit_lateral");
	if ((idx = get_str_index(table, size, "grid_rows")) >= 0)
		if(sscanf(table[idx].value, "%d", &config->grid_rows) != 1)
			fatal("invalid format for configuration  parameter grid_rows");
	if ((idx = get_str_index(table, size, "grid_cols")) >= 0)
		if(sscanf(table[idx].value, "%d", &config->grid_cols) != 1)
			fatal("invalid format for configuration  parameter grid_cols");
	if ((idx = get_str_index(table, size, "grid_layer_file")) >= 0)
		if(sscanf(table[idx].value, "%s", config->grid_layer_file) != 1)
			fatal("invalid format for configuration  parameter grid_layer_file");
	if ((idx = get_str_index(table, size, "grid_steady_file")) >= 0)
		if(sscanf(table[idx].value, "%s", config->grid_steady_file) != 1)
			fatal("invalid format for configuration  parameter grid_steady_file");
	if ((idx = get_str_index(table, size, "grid_map_mode")) >= 0)
		if(sscanf(table[idx].value, "%s", config->grid_map_mode) != 1)
			fatal("invalid format for configuration  parameter grid_map_mode");
	
	if ((config->t_chip <= 0) || (config->s_sink <= 0) || (config->t_sink <= 0) || 
		(config->s_spreader <= 0) || (config->t_spreader <= 0) || 
		(config->t_interface <= 0))
		fatal("chip and package dimensions should be greater than zero\n");
	if ((config->thermal_threshold < 0) || (config->c_convec < 0) || 
		(config->r_convec < 0) || (config->ambient < 0) || 
		(config->base_proc_freq <= 0) || (config->sampling_intvl <= 0))
		fatal("invalid thermal simulation parameters\n");
	if (strcasecmp(config->model_type, BLOCK_MODEL_STR) &&
		strcasecmp(config->model_type, GRID_MODEL_STR))
		fatal("invalid model type. use 'block' or 'grid'\n");
	if(config->grid_rows <=1 || config->grid_cols <= 1)
		fatal("grid rows and columns should both be greater than 1\n");
	if (strcasecmp(config->grid_map_mode, GRID_AVG_STR) &&
		strcasecmp(config->grid_map_mode, GRID_MIN_STR) &&
		strcasecmp(config->grid_map_mode, GRID_MAX_STR) &&
		strcasecmp(config->grid_map_mode, GRID_CENTER_STR))
		fatal("invalid mapping mode. use 'avg', 'min', 'max' or 'center'\n");
}

/* 
 * convert config into a table of name-value pairs. returns the no.
 * of parameters converted
 */
int32_t thermal_config_to_strs(thermal_config_t *config, str_pair *table, int32_t max_entries)
{
	if (max_entries < 23)
		fatal("not enough entries in table\n");

	sprintf(table[0].name, "t_chip");
	sprintf(table[1].name, "thermal_threshold");
	sprintf(table[2].name, "c_convec");
	sprintf(table[3].name, "r_convec");
	sprintf(table[4].name, "s_sink");
	sprintf(table[5].name, "t_sink");
	sprintf(table[6].name, "s_spreader");
	sprintf(table[7].name, "t_spreader");
	sprintf(table[8].name, "t_interface");
	sprintf(table[9].name, "ambient");
	sprintf(table[10].name, "init_file");
	sprintf(table[11].name, "init_temp");
	sprintf(table[12].name, "steady_file");
	sprintf(table[13].name, "sampling_intvl");
	sprintf(table[14].name, "base_proc_freq");
	sprintf(table[15].name, "dtm_used");
	sprintf(table[16].name, "model_type");
	sprintf(table[17].name, "block_omit_lateral");
	sprintf(table[18].name, "grid_rows");
	sprintf(table[19].name, "grid_cols");
	sprintf(table[20].name, "grid_layer_file");
	sprintf(table[21].name, "grid_steady_file");
	sprintf(table[22].name, "grid_map_mode");

	sprintf(table[0].value, "%lg", config->t_chip);
	sprintf(table[1].value, "%lg", config->thermal_threshold);
	sprintf(table[2].value, "%lg", config->c_convec);
	sprintf(table[3].value, "%lg", config->r_convec);
	sprintf(table[4].value, "%lg", config->s_sink);
	sprintf(table[5].value, "%lg", config->t_sink);
	sprintf(table[6].value, "%lg", config->s_spreader);
	sprintf(table[7].value, "%lg", config->t_spreader);
	sprintf(table[8].value, "%lg", config->t_interface);
	sprintf(table[9].value, "%lg", config->ambient);
	sprintf(table[10].value, "%s", config->init_file);
	sprintf(table[11].value, "%lg", config->init_temp);
	sprintf(table[12].value, "%s", config->steady_file);
	sprintf(table[13].value, "%lg", config->sampling_intvl);
	sprintf(table[14].value, "%lg", config->base_proc_freq);
	sprintf(table[15].value, "%d", config->dtm_used);
	sprintf(table[16].value, "%s", config->model_type);
	sprintf(table[17].value, "%d", config->block_omit_lateral);
	sprintf(table[18].value, "%d", config->grid_rows);
	sprintf(table[19].value, "%d", config->grid_cols);
	sprintf(table[20].value, "%s", config->grid_layer_file);
	sprintf(table[21].value, "%s", config->grid_steady_file);
	sprintf(table[22].value, "%s", config->grid_map_mode);

	return 23;
}

/* 
 * wrapper routines interfacing with those of the corresponding 
 * thermal model (block or grid)
 */

/* 
 * allocate memory for the matrices. for the block model, placeholder 
 * can be an empty floorplan frame with only the names of the functional 
 * units. for the grid model, it is the default floorplan
 */
RC_model_t *alloc_RC_model(thermal_config_t *config, flp_t *placeholder)
{
	RC_model_t *model= (RC_model_t *) calloc (1, sizeof(RC_model_t));
	if (!model)
		fatal("memory allocation error\n");
	if(!(strcasecmp(config->model_type, BLOCK_MODEL_STR))) {
		model->type = BLOCK_MODEL;
		model->block = alloc_block_model(config, placeholder);
		model->config = &model->block->config;
	} else if(!(strcasecmp(config->model_type, GRID_MODEL_STR))) {
		model->type = GRID_MODEL;
		model->grid = alloc_grid_model(config, placeholder);
		model->config = &model->grid->config;
	} else 
		fatal("unknown model type\n");
	return model;	
}

/* populate the thermal restistance values */
void populate_R_model(RC_model_t *model, flp_t *flp)
{
	if (model->type == BLOCK_MODEL)
		populate_R_model_block(model->block, flp);
	else if (model->type == GRID_MODEL)	
	/* ignore the flp parameter as it is not needed
	 * by the grid model which models a 3-d chip and
	 * takes its floorplan inputs from a layer 
	 * configuration file
	 */
		populate_R_model_grid(model->grid);
	else fatal("unknown model type\n");	
}

/* populate the thermal capacitance values */
void populate_C_model(RC_model_t *model, flp_t *flp)
{
	if (model->type == BLOCK_MODEL)
		populate_C_model_block(model->block, flp);
	else if (model->type == GRID_MODEL)	
	/* ignore the flp parameter as it is not needed
	 * by the grid model which models a 3-d chip and
	 * takes its floorplan inputs from a layer 
	 * configuration file
	 */
		populate_C_model_grid(model->grid);
	else fatal("unknown model type\n");	
}

/* steady state temperature	*/
void steady_state_temp(RC_model_t *model, double *power, double *temp) 
{
	if (model->type == BLOCK_MODEL)
		steady_state_temp_block(model->block, power, temp);
	else if (model->type == GRID_MODEL)	
		steady_state_temp_grid(model->grid, power, temp);
	else fatal("unknown model type\n");	
}

/* transient (instantaneous) temperature	*/
void compute_temp(RC_model_t *model, double *power, double *temp, double time_elapsed)
{
	if (model->type == BLOCK_MODEL)
		compute_temp_block(model->block, power, temp, time_elapsed);
	else if (model->type == GRID_MODEL)	
		compute_temp_grid(model->grid, power, temp, time_elapsed);
	else fatal("unknown model type\n");	
}

/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector(RC_model_t *model)
{
	if (model->type == BLOCK_MODEL)
		return hotspot_vector_block(model->block);
	else if (model->type == GRID_MODEL)	
		return hotspot_vector_grid(model->grid);
	else fatal("unknown model type\n");	
	return NULL;
}

/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector(RC_model_t *model, double *dst, double *src, 
						 int32_t at, int32_t size)
{
	if (model->type == BLOCK_MODEL)
		trim_hotspot_vector_block(model->block, dst, src, at, size);
	else if (model->type == GRID_MODEL)	
		trim_hotspot_vector_grid(model->grid, dst, src, at, size);
	else fatal("unknown model type\n");	
}

/* update the model's node count	*/						 
void resize_thermal_model(RC_model_t *model, int32_t n_units)
{
	if (model->type == BLOCK_MODEL)
		resize_thermal_model_block(model->block, n_units);
	else if (model->type == GRID_MODEL)	
		/* will be implemented only when grid model and HotFloorplan are integrated	*/
		fatal("function not implemented yet\n");
	else fatal("unknown model type\n");	
}

/* sets the temperature of a vector 'temp' allocated using 'hotspot_vector'	*/
void set_temp(RC_model_t *model, double *temp, double val)
{
	if (model->type == BLOCK_MODEL)
		set_temp_block(model->block, temp, val);
	else if (model->type == GRID_MODEL)	
		set_temp_grid(model->grid, temp, val);
	else fatal("unknown model type\n");	
}

/* dump temperature vector alloced using 'hotspot_vector' to 'file' */ 
void dump_temp(RC_model_t *model, double *temp, char *file)
{
	if (model->type == BLOCK_MODEL)
		dump_temp_block(model->block, temp, file);
	else if (model->type == GRID_MODEL)	
		dump_temp_grid(model->grid, temp, file);
	else fatal("unknown model type\n");	
}

/* 
 * read temperature vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_temp'. values are clipped to thermal
 * threshold based on 'clip'
 */ 
void read_temp(RC_model_t *model, double *temp, char *file, int32_t clip)
{
	if (model->type == BLOCK_MODEL)
		read_temp_block(model->block, temp, file, clip);
	else if (model->type == GRID_MODEL)	
		read_temp_grid(model->grid, temp, file, clip);
	else fatal("unknown model type\n");	
}

/* dump power numbers to file	*/
void dump_power(RC_model_t *model, double *power, char *file)
{
	if (model->type == BLOCK_MODEL)
		dump_power_block(model->block, power, file);
	else if (model->type == GRID_MODEL)	
		dump_power_grid(model->grid, power, file);
	else fatal("unknown model type\n");	
}

/* 
 * read power vector alloced using 'hotspot_vector' from 'file'
 * which was dumped using 'dump_power'. 
 */ 
void read_power (RC_model_t *model, double *power, char *file)
{
	if (model->type == BLOCK_MODEL)
		read_power_block(model->block, power, file);
	else if (model->type == GRID_MODEL)	
		read_power_grid(model->grid, power, file);
	else fatal("unknown model type\n");	
}

/* peak temperature on chip	*/
double find_max_temp(RC_model_t *model, double *temp)
{
	if (model->type == BLOCK_MODEL)
		return find_max_temp_block(model->block, temp);
	else if (model->type == GRID_MODEL)	
		return find_max_temp_grid(model->grid, temp);
	else fatal("unknown model type\n");	
	return 0.0;
}

/* average temperature on chip	*/
double find_avg_temp(RC_model_t *model, double *temp)
{
	if (model->type == BLOCK_MODEL)
		return find_avg_temp_block(model->block, temp);
	else if (model->type == GRID_MODEL)	
		return find_avg_temp_grid(model->grid, temp);
	else fatal("unknown model type\n");	
	return 0.0;
}

/* destructor	*/
void delete_RC_model(RC_model_t *model)
{
	if (model->type == BLOCK_MODEL)
		delete_block_model(model->block);
	else if (model->type == GRID_MODEL)	
		delete_grid_model(model->grid);
	else fatal("unknown model type\n");	
	free(model);
}
