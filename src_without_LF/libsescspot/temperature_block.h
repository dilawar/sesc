#ifndef __TEMPERATURE_BLOCK_H_
#define __TEMPERATURE_BLOCK_H_

#include "temperature.h"

/* block thermal model	*/
typedef struct block_model_t_st
{
	/* floorplan	*/
	flp_t *flp;

	/* configuration	*/
	thermal_config_t config;

	/* main matrices	*/
	/* conductance matrix */
	double **b;
	/* LUP decomposition of b	*/
	double **lu; 
	int32_t *p;
	/* diagonal capacitance matrix stored as a 1-d vector	*/
	double *a; 
	/* inverse of the above	*/
	double *inva;
	/* c = inva * b	*/
	double **c;

	/* thermal capacitance fitting factors	*/
	double factor_pack;	/* for package 	*/
	double factor_chip;	/* for silicon	*/
	double factor_inter;	/* for interface	*/

	/* intermediate vectors and matrices	*/
	double *gx, *gy, *gx_sp, *gy_sp, *t_vector;
	double **len, **g;
	int32_t **border;

	/* total no. of nodes	*/
	int32_t n_nodes;
	/* total no. of blocks	*/
	int32_t n_units;
	/* to allow for resizing	*/
	int32_t base_n_units;

	/* flags	*/
	int32_t r_ready;	/* are the R's initialized?	*/
	int32_t c_ready;	/* are the C's initialized?	*/
}block_model_t;

/* constructor/destructor	*/
/* placeholder is an empty floorplan frame with only the names of the functional units	*/
block_model_t *alloc_block_model(thermal_config_t *config, flp_t *placeholder);
void delete_block_model(block_model_t *model);

/* initialization	*/
void populate_R_model_block(block_model_t *model, flp_t *flp);
void populate_C_model_block(block_model_t *model, flp_t *flp);

/* hotspot main interfaces - temperature.c	*/
void steady_state_temp_block(block_model_t *model, double *power, double *temp);
void compute_temp_block(block_model_t *model, double *power, double *temp, double time_elapsed);
/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector_block(block_model_t *model);
/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector_block(block_model_t *model, double *dst, double *src, 
						 	   int32_t at, int32_t size);
/* update the model's node count	*/						 
void resize_thermal_model_block(block_model_t *model, int32_t n_units);						 
void set_temp_block (block_model_t *model, double *temp, double val);
void dump_temp_block (block_model_t *model, double *temp, char *file);
void read_temp_block (block_model_t *model, double *temp, char *file, int32_t clip);
void dump_power_block(block_model_t *model, double *power, char *file);
void read_power_block (block_model_t *model, double *power, char *file);
double find_max_temp_block(block_model_t *model, double *temp);
double find_avg_temp_block(block_model_t *model, double *temp);

#endif
