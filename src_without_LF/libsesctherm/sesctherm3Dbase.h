#ifndef SESCTHERM3D_BASE_H
#define SESCTHERM3D_BASE_H

class ucool_flp_unit;

class model_unit {
public:
	model_unit(){}
	model_unit(sesctherm3Ddatalibrary* datalibrary,
		   double conduct_up,
		   double conduct_right,
		   double conduct_down,
		   double conduct_left,
		   double conduct_bottom,
		   double conduct_top,
		   double conduct_center_up,
		   double conduct_center_right,
		   double conduct_center_down,
		   double conduct_center_left,
		   double conduct_center_bottom,
		   double conduct_center_top,
		   double resist_up,
		   double resist_right,
		   double resist_down,
		   double resist_bottom,
		   double resist_top,
		   double x1,double x2,double  x3,
		   double y1,double y2,double  y3,
		   double z1,double z2,double  z3,
		   double governing_equation);
	
	 SUElement_t* get_temperature();
	 SUElement_t* get_previous_temperature();
	static vector<double> compute_average_temps(int flp_layer, dynamic_array<model_unit>& dyn_array, int32_t sample_type, sesctherm3Ddatalibrary* datalibrary);
	static vector<double> compute_average_powers(int flp_layer, dynamic_array<model_unit>& dyn_array, int32_t sample_type, sesctherm3Ddatalibrary* datalibrary);
	static void zero(model_unit& model_unit, sesctherm3Ddatalibrary* datalibrary);
    model_unit(sesctherm3Ddatalibrary* datalibrary);
	double calc_view_factor(double width, double height, double distance);
	static void print_dyn_layer(int layer, sesctherm3Ddatalibrary* datalibrary_);
	static model_unit* find_model_unit(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary_);
	static void locate_model_units(int layer, 
				       vector<model_unit *>& located_units,
				       double leftx,
				       double bottomy,
				       double rightx,
				       double topy,
				       sesctherm3Ddatalibrary* datalibrary);
	
	static void locate_model_units(dynamic_array<model_unit>& dyn_array, 
								   vector<model_unit *>& located_units,
								   double leftx,
								   double bottomy,
								   double rightx,
								   double topy,
								   sesctherm3Ddatalibrary* datalibrary);
	
	double calc_alpha_max(bool use_rc);
	void accumulate_rc();
	void compute_rc();
	static int32_t find_model_unit_xitor(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary_);
	static int32_t find_model_unit_yitor(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary_);
	void set_power(double power);

	void operator=(int value) {
	}
  
    //calulate all the term values given the equation number
    void calculate_terms(double time_increment, bool calc_solution_only, bool use_rc);
    void calc_gamma(double time_increment, bool use_urc);
	//void calc_hot_spot_count();
    //void calc_hot_spot_duration();
    //calculate R_th/2 
    void calc_ucool_resistance_th();
	

	
	//data values
	int x_coord_;
	int y_coord_;
    sesctherm3Ddatalibrary* datalibrary_;
    ucool_flp_unit*   source_ucool_flp_; //this is a reference to the source ucooler flp that this modelunit is a part of
    chip_flp_unit*   source_chip_flp_; //this is a reference to the source chip flp that this modelunit is a part of
    bool defined_;          //is this modelunit used?
  //  string name_;            //name of region that unit belongs to 
    double flp_percentage_;      //percentage of the region's area that the unit belongs to. Note: this does need to be used when calculating energy data
    //as we are using energy/per unit area anyway. However, it does need to be used when calculating an average temperature
    //per flp
    double ucool_percentage_;   //percentage of the microcooler that the unit belongs to. This is used to calculate the proper resistances and internal
    //heat generation for each of the micro-coolers
    double ucool_rth_;          //this is Rth/2
    //power data
    double energy_data_;    //this is the energy data for the given call to the solver
    //solution data
	double lock_temp_;
	bool temp_locking_enabled_;
	
    SUElement_t** temperature_;			//this is the current temperature (points to another array)
    SUElement_t** previous_temperature_; //this was the previously calculated result (initially the steady-temp value)
	double sample_temperature_;	//this is a local storage of the temperature (only used in the graphics subsystem)
								//this is used to make the algorithm simpler
	int temperature_index_;
    //gamma
    double gamma_;
    //row
    double row_;

    //specific heat (Cp) or (C)
    double specific_heat_;

	double specific_heat_eff_;
	
    //material conductivities
    //m,n,o+1 (up)
    double conduct_up_;
    //m+1,n,o (right)
    double conduct_right_;
    //m,n,o-1 (down)
    double conduct_down_;
    //m-1,n,o (left)
    double conduct_left_;
    //m,n-1,o (bottom)
    double conduct_bottom_;
    //m,n+1,o (top)
    double conduct_top_;
	
	//These are the conductivities for the central region of the model unit
	//However, we have separate conductivities for each direction.
	//The effective conductivity would, of course, be the average of these values
	//However, we can obtain greater accurracy by splitting this up into separate values
	//This makes sense as the lateral resistance of a given layer may be very different than
	//the vertical resistance
	
	//m,n,o (up)
    double conduct_center_up_;
    //m,n,o (right)
    double conduct_center_right_;
    //m,n,o (down)
    double conduct_center_down_;
    //m,n,o (left)
    double conduct_center_left_;
    //m,n,o (bottom)
    double conduct_center_bottom_;
    //m,n,o (top)
    double conduct_center_top_;

    //interface resistances
    //m,n,o+1 (up)
    double resist_up_;
    //m+1,n,o (right)
    double resist_right_;
    //m,n,o-1 (down)
    double resist_down_;
    //m-1,n,o (left)
    double resist_left_;
    //m,n-1,o (bottom)
    double resist_bottom_;
    //m,n+1,o (top)
    double resist_top_;

    //dimensions
    double x1_;
    double x2_;
    double x3_;
    double y1_;
    double y2_;
    double y3_;
    double z1_;
    double z2_;
    double z3_;

	double leftx_;
	double rightx_;
	double topy_;
	double bottomy_;
	double width_;
	double height_;

//	double hot_spot_count_;
//	double hot_spot_duration_;
//	bool not_hot_spot_;
	
    //the governing equation for the given model unit
	//Equation 1 is basic heat transfer to surrounding solid or air flow
    //Equation 2 is for microcooler cold side
	//Equation 3 is for microcooler inner layer
	//Equation 4 is for microcooler hot side
	//Equation 5 is for chip pins
	//Equation 6 is for conducting heat through thermal grease
	//Equation 7 is for conducting heat through thermal crystals
	//Equation 8 is for conducting heat through saffire
	//Equation 9 is for conducting heat through water flow
	double governing_equation_;
	
	


	//the value for each of the terms for the governing equation
    // Tm,n,o
    SUElement_t* t_mno;
    // Tm-1,n,o
    SUElement_t* t_m_M1_no;
    // Tm+1,n,o
    SUElement_t* t_m_P1_no;
    // Tm,n-1,o	
    SUElement_t* t_mn_M1_o;
    // Tm,n+1,o	
    SUElement_t* t_mn_P1_o;
    // Tm,n,o-1	
    SUElement_t* t_mno_M1;
    // Tm,n,o+1	
    SUElement_t* t_mno_P1;
    // T_inf m,n,o	
    SUElement_t* tINF_mno;
    // T_inf m-1,n,o
    SUElement_t* tINF_m_M1_no;
    // T_inf m+1,n,o
    SUElement_t* tINF_m_P1_no;
    // T_inf m,n-1,o
    SUElement_t* tINF_mn_M1_o;
    // T_inf m,n+1,o
    SUElement_t* tINF_mn_P1_o;
    // T_inf m,n,o-1
    SUElement_t* tINF_mno_M1;
    // T_inf m,n,o+1
    SUElement_t* tINF_mno_P1;
    // Tvil,m,o
    SUElement_t* tVIL_mo;
	
	
    // Tm-1,n,o
    SUElement_t t_m_M1_no_eff;
    // Tm+1,n,o
    SUElement_t t_m_P1_no_eff;
    // Tm,n-1,o	
    SUElement_t t_mn_M1_o_eff;
    // Tm,n+1,o	
    SUElement_t t_mn_P1_o_eff;
    // Tm,n,o-1	
    SUElement_t t_mno_M1_eff;
    // Tm,n,o+1	
    SUElement_t t_mno_P1_eff;
    // T_inf m,n,o	
    SUElement_t tINF_mno_eff;
    // T_inf m-1,n,o
    SUElement_t tINF_m_M1_no_eff;
    // T_inf m+1,n,o
    SUElement_t tINF_m_P1_no_eff;
    // T_inf m,n-1,o
    SUElement_t tINF_mn_M1_o_eff;
    // T_inf m,n+1,o
    SUElement_t tINF_mn_P1_o_eff;
    // T_inf m,n,o-1
    SUElement_t tINF_mno_M1_eff;
    // T_inf m,n,o+1
    SUElement_t tINF_mno_P1_eff;
    // Tvil,m,o
    SUElement_t tVIL_mo_eff;
	
	
	model_unit* model_left_;
	model_unit* model_right_;
	model_unit* model_bottom_;
	model_unit* model_top_;
	model_unit* model_down_;
	model_unit* model_up_;
	
	//SUElement_t density_;
	SUElement_t power_per_unit_area_;
	
	//These are binary values, use DEFINES: HEAT_CONVECTION_TRANSFER, HEAT_CONDUCTION_TRANSFER, HEAT_EMMISION_TRANSFER for transfer types
	uint32_t heat_transfer_methods_left_;
	uint32_t heat_transfer_methods_right_;
	uint32_t heat_transfer_methods_down_;
	uint32_t heat_transfer_methods_up_;
	uint32_t heat_transfer_methods_top_;
	uint32_t heat_transfer_methods_bottom_;
	uint32_t heat_transfer_methods_center_;
	
	SUElement_t convection_coefficient_;
	SUElement_t emissivity_;
	
	//this is used for the sampling subsystem
	SUElement_t ave_temperature_;
	SUElement_t max_temperature_;
	SUElement_t min_temperature_;
	SUElement_t ave_energy_data_;
	SUElement_t max_energy_data_;
	SUElement_t min_energy_data_;
	
	vector<double> datapoints_x_;
	vector<double> datapoints_y_;
};


#endif

