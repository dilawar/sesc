#ifndef __TEMPERATURE_GRID_H_
#define __TEMPERATURE_GRID_H_

/* grid model differs from the block model in its use of
 * a mesh of cells whose resolution is configurable. unlike 
 * the block model, it can also model a stacked 3-D chip,
 * with each layer having a different floorplan. information
 * about the floorplan and the properties of each layer is
 * input in the form of a layer configuration file. 
 */
#include <fstream>
#include <vector>
#include <string>

#include "temperature.h"
#include "FBlocks.h"
#include "Grids.h"

/* grid model specific constants */
#define DATA_PER_LAYER 7		/* number of parameters per layer in the layer config file */

//This struct holds the floorplan details of a layer. This is calculated from
//the parameters read from the file. This is retained throughout the file.
struct FloorplanDetails {
        string Filename;
        vector <FBlocks> FBlockChip;
        vector <Grids> GridChip;
};

//This struct holds the properties of a layer. This is calculated from
//the parameters read from the file. This is retained throughout the file.
struct LayerDetails {
        int32_t Id;
        bool Lateral;
        bool Power;
        double Rx;
        double Ry;
        double Rz;
        double Cap;
        double Delta_temp;
		string flp_name;
};

/*
 * This struct holds the parameters of a layer read from the layer file.
 * The vector that holds these parameters is deleted once the program
 * leaves the function.
 */
struct layerprops {
	int32_t Id;
	bool Lateral;
	bool Power;
	double SpHtCap;
	double Rho;
	double K;
	double Thickness;
	string flp_name;
};

struct TempDetail {
	double cond;
	bool IsRed;
};

/* grid thermal model	*/
typedef struct grid_model_t_st
{
	/* default floorplan	*/
	flp_t *flp;
	
	/* configuration	*/
	thermal_config_t config;

	/* flags	*/
	int32_t r_ready;	/* are the R's initialized?	*/
	int32_t c_ready;	/* are the C's initialized?	*/

	/* default chip thickness */
	double t_chip;
	
	/* chip width and height */
	double chip_width;
	double chip_height;

	/* layer configuration file	*/
	string layer_cfg;

	/* Holds the list of layers */
	/* vector of LayerDetails vectors: for each multigrid level */
	vector <vector <LayerDetails> > Layers;
	vector <FloorplanDetails> Floorplan;

	//To hold the floorplan files
	vector <string> FloorplanFiles;

	/* This is an internal list of layers. This just holds the layer 
	 * parameters. Also has the spreader layer parameters as the last layer
	 */
	vector <layerprops> LayerInternal;
	
	/*dummy variable: multigrid levels */
	int32_t mv_level;

	/* thermal capacitance fitting factor for silicon	*/
	double factor_chip;

	double Rconv, Rsplex, Rspley, Rspl, Rspv, Rsi, Rsicv;
	double Csp, Csi, Csic, Cconv;

	double Tamb;

 	/* total number of nodes excuding the heatsink */
	int32_t total_n_blocks;
	/* sum total of the functional blocks of all floorplans	*/
	int32_t sum_n_units;
	/* grid resolution	*/
	int32_t row, col;
	/* grid-to-block mapping mode	*/
	int32_t map_mode;

	/* steady state and transient grid temperatures	*/
	vector <double> steady_g_temp, g_temp;
	/* grid power	*/
	vector <double> g_power;
}grid_model_t;

/* constructor/destructor	*/
grid_model_t *alloc_grid_model(thermal_config_t *config, flp_t *flp_default);
void delete_grid_model(grid_model_t *model);

/* initialization	*/
void populate_R_model_grid(grid_model_t *model);
void populate_C_model_grid(grid_model_t *model);

/* hotspot main interfaces - temperature.c	*/
void steady_state_temp_grid(grid_model_t *model, double *power, double *temp);
void compute_temp_grid(grid_model_t *model, double *power, double *temp, double time_elapsed);

/* read in layer config info for the grid model */
int32_t layer_R_setup(grid_model_t *model);
int32_t layer_C_setup(grid_model_t *model);

//Setup FBlockChip and GridChip
int32_t grid_setup (grid_model_t *model, double init_temp);

//initialize grid power in the grid model 
void init_gpower(int32_t layer_total, grid_model_t *model);

//Translate functional block temp to grid temp
void trans_btemp_gtemp(vector<double> &temp);

//Translates overall_power for functional blocks to overall_g_power for grid elements
void trans_bpow_gpow (vector<double> &overall_power, grid_model_t *model);

//Translates g_temp and steady_g_temp for grid elements to temp and steady_temp for functional blocks
void trans_tran_gtemp_btemp(vector<double> &temp, grid_model_t *model);
void trans_steady_gtemp_btemp(vector<double> &steady_temp, grid_model_t *model);

//Translates temp and steady_temp for functional blocks to g_temp and steady_g_temp for grid elements 
void trans_tran_btemp_gtemp(vector<double> &temp, grid_model_t *model);
void trans_steady_btemp_gtemp(vector<double> &temp, grid_model_t *model);

//Calculates the transient temperature, called by fast_tran_solver()
void compute_tran_temp(double time_elapsed, grid_model_t *model, vector<double> &temp, vector<double> &power);

//Calculates the steady-state temperature, called by fast_steady_solver()
void compute_steady_temp(grid_model_t *model, vector<double> &steady_temp, vector<double> &power);

//Solve transient temperature (fast solver)
void fast_tran_solver(double sampling_intvl, grid_model_t *model);

//Solve steady-state temperature (fast solver)
void fast_steady_solver(grid_model_t *model);

double SumTemp (vector<double> &temp, int32_t row, int32_t col, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t Offset, int32_t Offset2, int32_t Offset3);
void LayerTemperature (vector<double> &temp, vector<double> &power, grid_model_t *model, double delta_t);
double SteadySumTemp (vector<double> &steady_temp, int32_t row, int32_t col, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t Offset, int32_t Offset2, int32_t Offset3);
void LayerSteadyTemperature (vector<double> &steady_temp, vector<double> &power, grid_model_t *model);

/* differs from 'dvector()' in that memory for internal nodes is also allocated	*/
double *hotspot_vector_grid(grid_model_t *model);
/* copy 'src' to 'dst' except for a window of 'size'
 * elements starting at 'at'. useful in floorplan
 * compaction
 */
void trim_hotspot_vector_grid(grid_model_t *model, double *dst, double *src, 
						 	  int32_t at, int32_t size);
/* update the model's node count	*/						 
void resize_thermal_model_grid(grid_model_t *model, int32_t n_units);						 
void set_temp_grid (grid_model_t *model, double *temp, double val);
void dump_temp_grid (grid_model_t *model, double *temp, char *file);
void read_temp_grid (grid_model_t *model, double *temp, char *file, int32_t clip);
void dump_power_grid(grid_model_t *model, double *power, char *file);
void read_power_grid (grid_model_t *model, double *power, char *file);
double find_max_temp_grid(grid_model_t *model, double *temp);
double find_avg_temp_grid(grid_model_t *model, double *temp);

#endif
