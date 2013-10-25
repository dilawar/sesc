#ifndef __TEMPERATURE_H_
#define __TEMPERATURE_H_

#include "flp.h"
#include "util.h"

/* model type	*/
#define BLOCK_MODEL		0
#define GRID_MODEL		1
#define BLOCK_MODEL_STR	"block"
#define GRID_MODEL_STR	"grid"

/* grid to block mapping mode	*/
#define	GRID_AVG	0
#define	GRID_MIN	1
#define	GRID_MAX	2
#define	GRID_CENTER	3
#define	GRID_AVG_STR	"avg"
#define	GRID_MIN_STR	"min"
#define	GRID_MAX_STR	"max"
#define	GRID_CENTER_STR	"center"

/* number of extra nodes due to the model	*/
/* 5 spreader and 5 heat sink nodes (north, south, east, west and bottom)	*/
#define EXTRA	10
#define	SP_W	0
#define	SP_E	1
#define	SP_N	2
#define	SP_S	3
#define	SP_B	4
#define	SINK_W	5
#define	SINK_E	6
#define	SINK_N	7
#define	SINK_S	8
#define	SINK_B	9


/* functional block layers, block model only	*/
/* total	*/
#define NL	3
/* silicon is always layer 0	*/
/* interface layer, block model only	*/
#define IFACE	1
/* heat spreader, block model only	*/
#define HSP	2

/* physical constants	*/
#define RHO_SI	0.01	/* thermal resistivity of silicon between 300K-400K in (mK)/W	*/
#define	RHO_CU	0.0025	/* thermal resistivity of copper between 300K-400K in (mK)/W	*/
#define RHO_INT	0.75	/* thermal resistivity of the interface material in (mK)/W	*/
#define K_SI	(1.0/RHO_SI)	/* thermal conductivity of silicon	*/
#define K_CU	(1.0/RHO_CU)	/* thermal conductivity of copper	*/
#define K_INT	(1.0/RHO_INT)	/* thermal conductivity of the interface material	*/
#define SPEC_HEAT_SI	1.75e6	/* specfic heat of silicon in J/(m^3K)	*/
#define SPEC_HEAT_CU	3.55e6	/* specific heat of copper in J/(m^3K)	*/
#define SPEC_HEAT_INT	4e6		/* specific heat of the interface material in J/(m^3K)	*/

/* model specific constants	*/
#define C_FACTOR	0.5		/* fitting factor to match floworks (due to lumping)	*/

/* constants related to transient temperature calculation	*/
#define MIN_STEP	1e-7	/* 0.1 us	*/

/* thermal model configuration	*/
typedef struct thermal_config_t_st
{
	/* chip specs	*/
	double t_chip;	/* chip thickness in meters	*/
	double thermal_threshold;	/* temperature threshold for DTM (Kelvin)*/

	/* heat sink specs	*/
	double c_convec;	/* convection capacitance in J/K */
	double r_convec;	/* convection resistance in K/W	*/
	double s_sink;	/* heatsink side in meters	*/
	double t_sink;	/* heatsink thickness in meters	*/

	/* heat spreader specs	*/
	double s_spreader;	/* spreader side in meters	*/
	double t_spreader;	/* spreader thickness in meters	*/

	/* interface material specs	*/
	double t_interface;	/* interface material thickness in meters	*/

	/* others	*/
	double ambient;			/* ambient temperature in kelvin	*/
	/* initial temperatures	from file	*/
	char init_file[STR_SIZE];
	double init_temp;		/* if init_file is NULL	*/
	/* steady state temperatures to file	*/
	char steady_file[STR_SIZE];
	double sampling_intvl;	/* interval per call to compute_temp	*/
	double base_proc_freq;	/* in Hz	*/
	int32_t dtm_used;			/* flag to guide the scaling of init Ts	*/
	/* model type - block or grid */
	char model_type[STR_SIZE];

	/* parameters specific to block model	*/
	int32_t block_omit_lateral;	/* omit lateral resistance?	*/

	/* parameters specific to grid model	*/
	int32_t grid_rows;			/* grid resolution - no. of rows	*/
	int32_t grid_cols;			/* grid resolution - no. of cols	*/
	/* layer configuration from	file */
	char grid_layer_file[STR_SIZE];
	/* output grid temperatures instead of block temperatures */
	char grid_steady_file[STR_SIZE];
	/* mapping mode between grid and block models	*/
	char grid_map_mode[STR_SIZE];
}thermal_config_t;

/* defaults	*/
thermal_config_t default_thermal_config(void);
/* 
 * parse a table of name-value string pairs and add the configuration
 * parameters to 'config'
 */
void thermal_config_add_from_strs(thermal_config_t *config, str_pair *table, int32_t size);
/* 
 * convert config into a table of name-value pairs. returns the no.
 * of parameters converted
 */
int32_t thermal_config_to_strs(thermal_config_t *config, str_pair *table, int32_t max_entries);

/* hotspot thermal model - can be a block or grid model	*/
struct block_model_t_st;
struct grid_model_t_st;
typedef struct RC_model_t_st
{
	union
	{
		struct block_model_t_st *block;
		struct grid_model_t_st *grid;
	};
	/* block model or grid model	*/
	int32_t type;
	thermal_config_t *config;
}RC_model_t;

/* constructor/destructor	*/
/* placeholder is an empty floorplan frame with only the names of the functional units	*/
RC_model_t *alloc_RC_model(thermal_config_t *config, flp_t *placeholder);
void delete_RC_model(RC_model_t *model);

/* initialization	*/
void populate_R_model(RC_model_t *model, flp_t *flp);
void populate_C_model(RC_model_t *model, flp_t *flp);

/* hotspot main interfaces - temperature.c	*/
void steady_state_temp(RC_model_t *model, double *power, double *temp);
void compute_temp(RC_model_t *model, double *power, double *temp, double time_elapsed);
/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector(RC_model_t *model);
/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector(RC_model_t *model, double *dst, double *src, 
						 int32_t at, int32_t size);
/* update the model's node count	*/						 
void resize_thermal_model(RC_model_t *model, int32_t n_units);						 
void set_temp (RC_model_t *model, double *temp, double val);
void dump_temp (RC_model_t *model, double *temp, char *file);
void read_temp (RC_model_t *model, double *temp, char *file, int32_t clip);
void dump_power(RC_model_t *model, double *power, char *file);
void read_power (RC_model_t *model, double *power, char *file);
double find_max_temp(RC_model_t *model, double *temp);
double find_avg_temp(RC_model_t *model, double *temp);

/* other functions used by the above interfaces	*/

/* LUP decomposition	*/
void lupdcmp(double**a, int32_t n, int32_t *p, int32_t spd);

/* get the thermal resistance values	*/
double getr(double k, double Wb, double Lb, double Ws, double t);

/* LU forward and backward substitution	*/
void lusolve(double **a, int32_t n, int32_t *p, double *b, double *x, int32_t spd);

/* 4th order Runge Kutta solver with adaptive step sizing */
double rk4(double **c, double *y, double *p, int32_t n, double h, double *yout);

/* matrix routines	*/
void matmult(double **c, double **a, double **b, int32_t n);
/* same as above but 'a' is a diagonal matrix stored as a 1-d array	*/
void diagmatmult(double **c, double *a, double **b, int32_t n); 
void matvectmult(double *vout, double **m, double *vin, int32_t n);
/* same as above but 'm' is a diagonal matrix stored as a 1-d array	*/
void diagmatvectmult(double *vout, double *m, double *vin, int32_t n);
/* 
 * inv = m^-1, inv, m are n by n matrices.
 * the spd flag indicates that m is symmetric 
 * and positive definite 
 */
void matinv(double **inv, double **m, int32_t n, int32_t spd);

#endif
