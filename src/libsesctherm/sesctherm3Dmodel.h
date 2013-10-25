#ifndef SESCTHERM3D_MODEL_H
#define SESCTHERM3D_MODEL_H


class sesctherm3Dmodel;

class sesctherm3Dmodel{
private:
	dynamic_array<model_unit>& get_floorplan_layer(int layer);
	void print_unsolved_model();
	void check_matrices();
	void print_unsolved_model_row(int row);
	void copy_solution_to_floorplan_superlu(SuperMatrix& A);
	void fill_solution_matrix_superlu(SUElement_t* solution_matrix);
	void set_initial_temperatures();
	void create_solution_matrices();
	void createSuperLUMatrix();
	void solve_matrix_superlu();	
	inline int32_t  find_unsolved_matrix_row_index(int layer, int32_t x_coord, int32_t y_coord);
	
	
	//void store_non_zero(int x_coord, int32_t y_coord, double& value);
	//void create_unsolved_matrix_row(int y_itor_global, int32_t layer, int32_t x_itor, int32_t y_itor);
	//void create_unsolved_matrix();
	
	void find_unused_model_units();
	void store_pointer(int x_coord, int32_t y_coord, SUElement_t*& pointer_location, bool set_pointer);
	void initialize_unsolved_matrix_row(int y_itor_global, int32_t layer, int32_t x_itor, int32_t y_itor, bool set_pointer);
	void initialize_unsolved_matrix();
	
	void compute_model_units(bool calc_solution_only, double time_increment, bool accumulate_rc, bool use_rc);
	void partition_floorplans();
	double get_governing_equation(double rightx, double topy, double width, double height, int32_t layer);
	uint32_t determine_heat_transfer_methods(model_unit& model_unit_source,model_unit& model_unit_dest);
	void store_model_unit(double rightx, double topy, double width, double height, int32_t x_itor, int32_t y_itor, int32_t layer);
	void generate_pins_layer(double width, double height);
	void recompute_material_properties();
	void recompute_material_properties(int layer);
	void run_sampler(double timestep, bool use_rc, bool recompute_material_properties);
public:
	sesctherm3Dmodel(const char* flp_filename=NULL, 									
			 const char* ucool_filename=NULL, 
			 const char* output_filename="sesctherm.out",
			 bool get_chipflp_from_sescconf=true, 
			 bool get_ucoolflp_from_sescconf=false);
	~sesctherm3Dmodel();
	
	void compute(double timestep, bool recompute_material_properties);
	void compute(bool recompute_material_properties);	
	void compute_rc_start();
	void compute_rc_stop();
	void fast_forward(double timestep);
	void set_time(double timestep);
	void determine_gridsize();
	void determine_timestep();
	void print_graphics();
	void accumulate_sample_data();																		//
	void compute_sample();																				//
	void set_power_flp(int flp_pos, int32_t layer, double power);											//
	void set_power_layer(int layer, vector<double>& power_map);											//
	void set_temperature_flp(int flp_pos, int32_t flp_layer, int32_t source_layer, double temperature);			//
	void set_temperature_layer(int flp_layer, int32_t source_layer, vector<double>& temperature_map);		//
	void set_temperature_layer(int source_layer, double temperature);									//
	void enable_temp_locking(int layer, double temperature);
	void disable_temp_locking(int layer);
	vector<double> get_temperature_layer(int flp_layer, int32_t source_layer);								//
	double get_temperature_flp(int flp_pos, int32_t flp_layer, int32_t source_layer);							//
	vector<double> get_power_layer(int flp_layer);														//
	double get_power_flp(int flp_layer, int32_t flp_pos);													//
	void set_ambient_temp(double temp);
	void set_initial_temp(double temp);
	double get_time();
	double get_max_timestep();
	double get_recommended_timestep();
	uint32_t chip_flp_count(int layer);
	string get_chip_flp_name(int flp_pos, int32_t layer);
	void set_max_power_flp(int flp_pos, int32_t layer, double power);
	void dump_temp_data(); // FIXME: rename dump
	void run_model(double timestep, bool accumulate_rc, bool use_rc, bool recompute_material_properties);
	int get_modelunit_count(int layer);
	int get_modelunit_rows(int layer);
	int get_modelunit_cols(int layer);
	
	void dump_flp_temp_data(); // TODO
	void print_flp_temp_labels(); // TODO
	void warmup_chip(); // TODO
	void dump_3d_graph(); // TODO
	void print_model_units();
	
	
	dynamic_array<model_unit> &get_dyn_array(int layer);
	 
	//this contains all of the data 
	sesctherm3Ddatalibrary* datalibrary_;
	
	
	double time_;
	double time_step_;
	double cur_sample_end_time_;
	
	vector<double>   leftx_;         //holds compiled list of leftx_values
	vector<double>   bottomy_;        //holds compiled list of bottomy_values
};



#endif
