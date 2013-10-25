#ifndef _SESCTHERM3D_CONFIG_H
#define _SESCTHERM3D_CONFIG_H
class config_data;

class config_data {
public:
    config_data(sesctherm3Ddatalibrary* datalibrary_);
    void get_config_data();
    bool config_data_defined_;
    friend ostream& operator<<(ostream& os, config_data& configuration);
    //materials
	sesctherm3Dmaterial_list material_list;
	//std::vector <Material*> layer_material_;
	 //   Material* layer1_material_;
	 //   Material* layer2_material_;
	 //   Material* layer3_material_;
	 //   Material* layer4_material_;
	 //   Material* layer5_material_;

	 //   layer thicknesses
	 //std::vector <double> layer_thickness_;
	 //   double layer1_thickness_;
	 //   double layer2_thickness_;
	 //	  double layer3_thickness_;
	 //   double layer4_thickness_;
	 //   double layer5_thickness_;
	 
	 
	 //GET BASIC PROPERTIES
	double init_temp_;
	double ambient_temp_;
	double default_time_increment_;
	
    //GET COOLING PROPERTIES
	bool	fan_used_;
	double fan_velocity_;
	
	bool oil_used_;
	double oil_layer_convection_coefficient_;
	double coolant_density_;
	double coolant_viscosity_;
	double coolant_thermal_conductivity_;
	double coolant_specific_heat_;
	double coolant_prandtl_number_;
	double coolant_flow_rate_;
	double coolant_angle_;
	double coolant_nozzle_diameter_;
	double coolant_coverage_percent_;
	
	
	//GET THE CHIP PARAMETERS

	double transistor_count_;
	double pin_count_;
	double pins_height_;
	double pin_pitch_;
	double chip_width_;
	double chip_height_;
	double chip_thickness_;
	double package_height_;
	double package_width_;
	double package_thickness_;
		
	//GET THE HEAT SINK/SPREADER PROPERTIES
 
	double heat_spreader_height_; 
	double heat_spreader_width_; 
	double heat_spreader_thickness_;

	double heat_sink_height_; 
	double heat_sink_width_;   
	double heat_sink_thickness_;
	
	double heat_sink_fins_thickness_;
	
	double heat_spreader_microhardness_;
	double heat_spreader_surfaceroughness_;
	
	int heat_sink_fin_number_;
	double heat_sink_fins_thicknsss_;
	
	double heat_sink_resistance_;
	
	double heat_sink_microhardness_;
	double heat_sink_surfaceroughness_;
	double heat_sink_contactpressure_;

	//GET THE UCOOL PARAMETERS
    
	double ucool_width_;
	double ucool_height_;
	
	double ucool_current_;
	int ucool_coupled_devices_;
	

	double ucool_resistivity_;
	double ucool_conductivity_; // FIXME: not initialized
	double ucool_seebeck_;
	double ucool_g_;
	
	//interface material properties
    double interface_material_conductivity_;
    double interface_material_gasparameter_;
    
    double hot_spot_temp_;
    //FIXME: ucooler on temperature
    //FIXME: ucooler off temperatre 
	
	
	bool get_chipflp_from_sescconf_;
	bool get_ucoolflp_from_sescconf_;
	bool generate_ucool_floorplan_;
	
	double gen_flp_ucool_pwr_percentage_;
	double max_total_power_;
	double gen_flp_pwr_per_cooler_;
	bool gen_flp_calc_num_coolers_;
	int n_coolers_;
	int ucool_generation_policy_;
	double ucool_weighted_region_size_;
	double ucool_weighted_region_max_temp_;
	double ucool_weighted_region_min_temp_;
	double ucool_on_off_region_size_;
	int num_pins_;
	int pin_width_;
	int pin_height_;
	sesctherm3Ddatalibrary* datalibrary_;
	
	int num_processors_;		//number of threads to run
	int matrix_library_;		//matrix library selection (currently either MTL or SUPERLU)
	
	
	//regression parameters
	
	int regression_num_iterations_;
	
	//these are technology parameters
	int technology_;			//TECH_65, TECH_90, TECH_180, TECH_250
	double num_gates_;			//number of gates in design (defaults to 250M)
	
	map<double,map<int,double> > inter_layer_conductivities_;
	//interconnect parameters (for metal layers)
	//stochastic wire model parameters:
//	double wire_stochastic_p_;
//	double wire_stochastic_k_;
//	double wire_stochastic_fanout_;
//	int num_metal_layers_;		//number of metal layers (default 12)
	
	//parameters for calculating polysilicon thermal conductivity
//	double polysilicon_grain_size_;
//	double polysilicon_impurity_concentration_;
	
	
	//These parameters were taken from the following paper:
	// "Prediction of Thermal Performance of Flip Chip -- Plastic Ball Grid Array (FC_PBGA) Packages:
	//		Effect of Substrate Physical Design"
	//	K. Ramakrishna, T-Y. Tom Lee
	

	//FC-PBGA package substrate with C5 layer

	//FC-PBGA layer thicknesses
	//TopSM,M1, BT1, M2, BT2, M3, BT Core, M4, BT4, M5, BT5, M6, BottomSM, EQ C5 Layer
	double fcpbga_layer_total_thickness_;
	std::vector<double> fcpbga_layer_thickness_;
	std::vector<double> fcpbga_layer_conductivity_vert_;
	std::vector<double> fcpbga_layer_conductivity_horiz_;
	std::vector<double> fcpbga_layer_specific_heat_;
	std::vector<double> fcpbga_layer_density_;
	
	//PWB parameters
	//TopSM, M1, FR4 dielectric, M2, FR4, M3, FR4, M3, FR4, BottomSM
	double pwb_layer_total_thickness_;
	std::vector<double> pwb_layer_thickness_;
	std::vector<double> pwb_layer_conductivity_vert_;	
	std::vector<double> pwb_layer_conductivity_horiz_;
	std::vector<double> pwb_layer_specific_heat_;
	std::vector<double> pwb_layer_density_;

	
	//Sample Transistor Parameters
	double sample_oxide_thickness_;
	double sample_gate_length_;
	double sample_fox_length_;
	double sample_eff_channel_length_;
	double sample_contact_length_;
	double sample_silicon_substrate_thickness_;
	double sample_silicon_film_length_;
	double sample_silicon_film_thickness_;
	double sample_box_thickness_;
	double sample_vthreshold_;
	double sample_oxide_conductivity_;
	double sample_oxide_conductivity_transistor_;
	double sample_silicon_island_conductivity_;
	double sample_gate_width_;
	double sample_thermal_capacity_silicon_;
	double sample_thermal_capacity_oxide_;
	
	// Sample Parameters
	bool enable_samples_;
	double sample_start_time_;
	double sample_end_time_;
	double sample_duration_;
	
	
	//Graphics parameters
	bool enable_graphics_;
	int resolution_x_;
	int resolution_y_;
	bool enable_perspective_view_;
	int  graphics_floorplan_layer_;
	//this holds all the parameters for file types:
	std::vector<sesctherm3Dgraphicsfiletype*> graphics_file_types_;	
	
	friend ostream& operator<<(ostream & os, const config_data& configuration){
	  os << "************ CONFIGURATION_DATA ************" << endl;
	  os << "Configuration Data Defined?:" << ((configuration.config_data_defined_==true)? "yes!" : "no") << endl;
	
    //materials
    //    for (uint32_t i=0; i<configuration.materials_.size(); i++)
    //        os << configuration.materials_[i] << endl;
	
	
    //Heat Spreader Dimensions
    os << "heat_spreader_height_ =" << configuration.heat_spreader_height_<< endl;
    os << "heat_spreader_width_ =" << configuration.heat_spreader_width_<< endl;
    //Heat Sink Dimensions
    os << "heat_sink_height_ =" << configuration.heat_sink_height_<< endl;
    os << "heat_sink_width_ =" << configuration.heat_sink_width_<< endl;
    //Number of Heat Sink Fins
    os << "heat_sink_fin_number_ =" << configuration.heat_sink_fin_number_<< endl;
    //Heat Sink Interface Resistance;
    os << "heat_sink_resistance_ =" << configuration.heat_sink_resistance_<< endl;
        //initial temp
    os << "init_temp_ =" << configuration.init_temp_<< endl;
    //time increment
	 os << "default_time_increment_ =" << configuration.default_time_increment_<< endl;
    //fan settings
    os << "fan_velocity_ =" << configuration.fan_velocity_ << endl;
    //ambient temp
    os << "ambient_temp_ =" << configuration.ambient_temp_ << endl;
    //micro-cooler settings
    os << "ucool_width_ =" << configuration.ucool_width_<< endl;
    os << "ucool_height_ =" << configuration.ucool_height_<< endl;
    os << "ucool_current_ =" << configuration.ucool_current_<< endl;
    os << "ucool_coupled_devices_ =" << configuration.ucool_coupled_devices_<< endl;
    os << "ucool_conductivity_ =" << configuration.ucool_conductivity_<< endl;
    os << "ucool_resistivity_ =" << configuration.ucool_resistivity_<< endl;
    os << "ucool_seebeck_ =" << configuration.ucool_seebeck_<< endl;
    os << "ucool_g_ =" << configuration.ucool_g_<< endl;
    return(os);
}

};
#endif
