#ifndef _SESCTHERM3D_DATALIBRARY_H
#define _SESCTHERM3D_DATALIBRARY_H

//This is the main class that allows access to all of the other data elements
//This is done to ensure that the data is properly maintained, and to ensure
//That any new class may be easily able to access any of the other data elements
//Important: while all of the other classes may be able to access these elements, 
//none have been declared global
class sesctherm3Ddatalibrary;
class layerinfo;
class RegressionLine;


class sesctherm3Ddatalibrary{
public:
	sesctherm3Ddatalibrary();
	~sesctherm3Ddatalibrary();
	double timestep_;			//this is the global timestep
	double time_;				//this is the global time of the model
	double cur_sample_end_time_;	//this is the endtime of the current sample (to know where it ends)
	const static double technology_parameters_[91][5];
	static int32_t tech_metal_index(int layer);
	std::ifstream if_flpfile_;
	std::ifstream if_cfgfile_;
	std::ifstream if_ucool_flpfile_;
	std::ofstream of_outfile_;

	
	//SuperLU unsolved matrix
	//this is the SuperLU format for the LU-decomposed unsolved matrix (computed only once)
	SuperMatrix A_;
	SuperMatrix L_superlu_;
	SuperMatrix U_superlu_;
	SuperMatrix B_superlu_;
	int * permr_superlu_;
	int * permc_superlu_;
	int * etree_superlu_;
	int info_superlu_;
#ifdef USE_MP
	Gstat_t	Gstat_superlu_;
#else
	SuperLUStat_t Gstat_superlu_;
#endif
	SUElement_t* rhs_superlu_;
	int rhs_size_superlu_;
	
	
	//MTL LU-decomposed unsolved matrix
	MatrixDense * LU_mtl_;
	dense1D<int> * pvector_mtl_;
	Vector solution_matrix_;
	SUElement_t* non_zero_vals_;
	int* row_indices_;
	int* col_begin_indices_;
	
	//Configuration data
	config_data* config_data_;	//changed "chip_configuration" to "config_data" to be consistent
		
	//Graphics Data
	sesctherm3Dgraphics* graphics_;
	
	//Reliability data
	//sesctherm_reliability* reliability_data_;
	
	//Performance data
	//performance_data* performance_data_;
	
	//Interconnect Model
	//sesctherm3Dinterconnect* interconnect_model_;
	
	//Materials Library
	sesctherm3Dmaterial_list layer_materials_;
	
	//this array holds the pointers to the dynamic floorplans
	//the allocation of these dynamic floorplans depends upon the defined layers
	//Layer Models
	//This stores ALL the relevent information for that particular model layer
	//depending upon the layer type, different information may be stored
	std::vector <sesctherm3Dlayerinfo*>				all_layers_info_;





	uint32_t chip_to_spreader_offset_x_;        //this is the x-distance between the first chip-layer model_unit and first heat-spreader model_unit
	uint32_t chip_to_spreader_offset_y_;        //this is the y-distance between the first chip-layer model_unit and first heat-spreader model_unit
	uint32_t spreader_to_sink_offset_x_;        //this is the x-distance between the first heat-spreader model_unit and first heat-sink model_unit
	uint32_t spreader_to_sink_offset_y_;        //this is the y-distance between the first heat-spreader model_unit and first heat-sink model_unit
	
	//int virtual_layer_unused_count_;            //the number of model units in the virtual layer that are unused
	//int heat_sink_fins_unused_count_;           //the number of model units in the heat sink fins layer that are unused
	//int pins_layer_unused_count_;				//the number of model units in the pins layer that are unused
	
	//UNUSED VARIABLES (remove?)
//	double heat_spread_width_;
//	double heat_spread_height_;
//	double heat_spread_area_;
//	double heat_sink_width_;
//	double heat_sink_height_;
//	double heat_sink_area_;
	
	SUElement_t* temperature_matrix_;				//this stores the solved temperatures for each iteration (each model unit links into this)
	SUElement_t* previous_temperature_matrix_;		//this stores the previously solved temperatures for each iteration (each model unit links into this)
	int temperature_matrix_size_;
	
	//dynamic_array<SUElement_t>* unsolved_matrix_dyn_;
	Matrix*	unsolved_matrix_dyn_;			//sparse unsolved Matrix (used for LU-factorization, then solving the system)
	int num_locktemp_rows_;
	
	//UNUSED VARIABLES (remove?)
	//dynamic_array<SUElement_t>* solved_matrix_dyn_;
	//SUElement_t * unsolved_array_;       //this is the simple array form of the dynamic unsolved array
	//SUElement_t * solution_array_;       //this is the simple array form of the solution array

		
	ofstream of_outfile_floorplan_; //the SVG to draw the floorplan
									 //Note the floorplan file can have a top-down view of a single floorplan, 
									 //or it can have both the chip and the ucooler floorplan
									 //or it can output the logical units from each of the layers
	//ofstream of_outfile_map_;		//the SVG to draw the thermal/power map 
	
	
	bool accumulate_rc_;
	bool use_rc_;
	//Non-linear Regression Variables
	double regression_opts_[LM_OPTS_SZ];
	double regression_info_[LM_OPTS_SZ];
};

#endif
