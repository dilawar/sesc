//----------------------------------------------------------------------------
// File: sesctherm3Dmodel.cpp
//
// Description: 3-Dimensional Thermal Model with Micro-cooler implementation
// Authors    : Joseph Nayfach - Battilana
//
//Note: Because of errors in the doubles data type, we are rounding all numbers to 1.0e-9


#include "sesctherm3Dinclude.h"


using namespace std;
using namespace mtl;


sesctherm3Dmodel::~sesctherm3Dmodel() {
	
    
	
	
	bool get_chipflp_from_sescconf=datalibrary_->config_data_->get_chipflp_from_sescconf_;
	bool get_ucoolflp_from_sescconf=datalibrary_->config_data_->get_ucoolflp_from_sescconf_;
	
	
    //open chip floorplan file
    if(!get_chipflp_from_sescconf){
		datalibrary_->if_flpfile_.close();
    }
	if(!get_ucoolflp_from_sescconf){
		datalibrary_->if_ucool_flpfile_.close();
    }
	datalibrary_->if_cfgfile_.close();
    datalibrary_->of_outfile_.close();
}

sesctherm3Dmodel::sesctherm3Dmodel(const char* flp_filename,
								   const char* ucool_flp_filename, 
								   const char* outfile_filename,
								   bool get_chipflp_from_sescconf, 
								   bool get_ucoolflp_from_sescconf) {
	
	
	
	datalibrary_ = new sesctherm3Ddatalibrary();
	time_=0;
	time_step_=0;
	cur_sample_end_time_=0;
	
	//store settings to datalibrary
	
	
	datalibrary_->config_data_->get_chipflp_from_sescconf_=get_chipflp_from_sescconf;
	datalibrary_->config_data_->get_ucoolflp_from_sescconf_=get_ucoolflp_from_sescconf;
	
	
    //open chip floorplan file
    if(!get_chipflp_from_sescconf){
	    datalibrary_->if_flpfile_.open(flp_filename,std::ifstream::in);
		
	    if (!datalibrary_->if_flpfile_) {
	        sesctherm_utilities::fatal("Cannot open floorplan file\n");
	    }
    }
	if(!get_ucoolflp_from_sescconf && ucool_flp_filename!=NULL){
	    //open ucooler floorplan file
	    datalibrary_->if_ucool_flpfile_.open(ucool_flp_filename, std::ifstream::in);
		
	    if (!datalibrary_->if_ucool_flpfile_) {
	        sesctherm_utilities::fatal("Cannot open floorplan file\n");
	    }
	}
	
	
	
    //create output file
	if(outfile_filename==NULL)
		outfile_filename="sesctherm.out";
    datalibrary_->of_outfile_.open(outfile_filename, std::ifstream::out);
    if (!datalibrary_->of_outfile_) {
        sesctherm_utilities::fatal("Cannot create output file\n");
    }
	
    //open the configuration file, store the information to config_data structure
    //this data will always be gotten from sesc.conf
    datalibrary_->config_data_->get_config_data();
    
#ifdef _SESCTHERM_DEBUG
	//	cerr << endl;
	//    cerr << *(datalibrary_->config_data_);
	//	cerr << endl;
#endif
	
	
	//determine all the properties for the given layer, load chip floorplans/ucooler floorplans
	//acquire ALL data necessary for each layer (material properties computation etc etc)
    for (int i=0;i<(int)datalibrary_->all_layers_info_.size();i++) {
		datalibrary_->all_layers_info_[i]->determine_layer_properties();
	}
	
	//Allocate space for the layers dynamic arrays
	//Also determine the leftx,rightx,bottomy,topy coordinates for the layers
	//This must be done after the layer widths and height have been determined
	//Hence, this must come after the layer properties have been determined
	sesctherm3Dlayerinfo::allocate_layers(datalibrary_);
	
	
	//Now that leftx/bottomy datapoints have been determined for each layer, offset the flp units accordingly
    for (int i=0;i<(int)datalibrary_->all_layers_info_.size();i++) {
		datalibrary_->all_layers_info_[i]->offset_layer();
	}
	
	
	
    cerr << "******** Layer Properties Determined, Printing Basic Layer Information: ******** " << endl;
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		datalibrary_->all_layers_info_[i]->print(false);	//don't print detailed information (the dynamic arrays)
	}	
	
	
	//partition the floorplans into different sections
    partition_floorplans();
	find_unused_model_units();             //determine the number of unused model-units in the virtual and heat_sink_fins layers
	
	create_solution_matrices();			   //allocate SUElement_t* temperature_matrix and SUElement_t* previous_temperature_matrix
										   //then associate the *solution_data and *previous_solution_data links to elements in these arrays
	set_initial_temperatures();
	recompute_material_properties();
	initialize_unsolved_matrix();			//this will set the coefficient pointers in the model units to point to the correct location in the unsolved matrix
	
	compute_model_units(false, datalibrary_->config_data_->default_time_increment_, false, false);            //calculate all the data for the model units.
	cerr << "******** Thermal Model Generated, Printing Lumped Resistance, Capacitance and RC-time constant for each layer: ********" << endl;
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		datalibrary_->all_layers_info_[i]->print_lumped_metrics();	
	}				
	
	
	
	//and store that data to unsolved matrix	
#ifdef _SESCTHERM_DEBUG
	/*
	 cerr << "******** Layers Have Been Partitioned, Printing Dynamic Layers: ********" << endl;
	 for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++)
	 if(datalibrary_->all_layers_info_[i]->layer_used_)
	 model_unit::print_dyn_layer(i, datalibrary_);
	 */ 
	print_unsolved_model();
	//NOTE: in order for the check_matrices feature to work, temp-locking must be set to 1C (and actually needs to be completely turned off for part of the checking)
	check_matrices();
#endif
}


//This allocates the solution_data and previous_solution_data matrices
//Then it links the *solution_data and *previous_solution_data pointers in each model unit to a location in each array
//These arrays will be swapped on each iteration (previous_solution_data=solution_data)
void sesctherm3Dmodel::create_solution_matrices(){
	//make sure that we haven't already allocated these matrices
	if(datalibrary_->temperature_matrix_!=NULL || datalibrary_->previous_temperature_matrix_!=NULL)
		return;
	
	int size=0;	
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++){
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++){ 
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					size++;
				}
		}
	}		
		
		
		datalibrary_->temperature_matrix_= new SUElement_t[size];
		datalibrary_->previous_temperature_matrix_= new SUElement_t[size];
		
		//Note: need to create last entry (where last entry is equal to T_ambient, as T_inf must be equal to T_ambient)
		int y_itor_global=0;
		for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
			if(datalibrary_->all_layers_info_[layer]->layer_used_){
				for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
					for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
						if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
							continue;   //skip if unit is not defined
						get_dyn_array(layer)[x_itor][y_itor].temperature_index_=y_itor_global;
						get_dyn_array(layer)[x_itor][y_itor].temperature_=&(datalibrary_->temperature_matrix_);
						get_dyn_array(layer)[x_itor][y_itor].previous_temperature_=&(datalibrary_->previous_temperature_matrix_);
						y_itor_global++;
					}
			}
				
				datalibrary_->temperature_matrix_size_=y_itor_global;
}

//this can ONLY be called once the temperature and previous_temperature matrices have been allocated
void sesctherm3Dmodel::set_initial_temperatures(){
	int y_itor_global=0;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					if(datalibrary_->all_layers_info_[layer]->temp_locking_enabled_){
						*get_dyn_array(layer)[x_itor][y_itor].get_previous_temperature()=datalibrary_->all_layers_info_[layer]->lock_temp_;
						*get_dyn_array(layer)[x_itor][y_itor].get_temperature()=datalibrary_->all_layers_info_[layer]->lock_temp_;
					}
					else{
						*get_dyn_array(layer)[x_itor][y_itor].get_previous_temperature()=datalibrary_->config_data_->init_temp_;
						*get_dyn_array(layer)[x_itor][y_itor].get_temperature()=datalibrary_->config_data_->init_temp_;	
					}
				}
		}
}


//this ensures that all of the equations were properly stored in memory
//this is done by setting the heat generation to zero in all units, and setting the previous temperatures to 1.
//this means that no heat is moving between units and the temperature at Tmno_{p+1}==Tmno_{p}
//If there is no error, then both sides of the equation will be equal
void sesctherm3Dmodel::check_matrices(){
	bool error=false;
	
	//set heat generation to zero in all layers and set the layer temperatures to one
	for(uint32_t j=0;j<datalibrary_->all_layers_info_.size();j++){
		set_temperature_layer(j,1.0);		//set the temperature to one
		if(datalibrary_->all_layers_info_[j]->chip_floorplan_!=NULL){
			for(uint32_t i=0;i<chip_flp_count(j);i++) {
				set_power_flp(i, j,  0.0); //set the power generation to zero
			}
		}
	}
	
	
	//recompute the material properties
	recompute_material_properties();
	
	
	
	//recompute the matrix
	int y_itor_global=0;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if(!get_dyn_array(layer)[x_itor][y_itor].defined_)
						continue;
					
					get_dyn_array(layer)[x_itor][y_itor].calculate_terms(datalibrary_->config_data_->default_time_increment_, false, false);
				}
		}	
			
			
			cerr << "******** Checking unsolved_matrix_dyn_ and temperature_matrix: ********" << endl;
			cerr << "NOTE: IN ORDER FOR THIS TO WORK, LOCK_TEMP MUST EITHER BE DISABLED FOR ALL LAYERS OR ELSE THE LOCK_TEMP VALUE MUST BE SET TO 1" << endl;
			cerr << "unsolved_matrix_dyn_.ncols()=" << datalibrary_->unsolved_matrix_dyn_->ncols() << endl;
			cerr << "unsolved_matrix_dyn_.nrows()=" << datalibrary_->unsolved_matrix_dyn_->nrows() << endl << endl;
			
			//Now we create the labels for all of the various units
			vector<string> tempVector;
			vector<int> vector_x, vector_y, vector_layer;
			tempVector.push_back("0,");
			for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
				if(datalibrary_->all_layers_info_[layer]->layer_used_)
					for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
						for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
							if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
								continue;   //skip if unit is not defined
							tempVector.push_back("L" + sesctherm_utilities::stringify(layer) + "[" + sesctherm_utilities::stringify(x_itor) + "][" + sesctherm_utilities::stringify(y_itor) + "],");
							vector_x.push_back(x_itor);
							vector_y.push_back(y_itor);
							vector_layer.push_back(layer);
						}
							//for(uint32_t itor=0;itor<tempVector.size();itor++)
							//	cerr << tempVector[itor];
							cerr << endl;
			
			cerr << "******** Checking For Proper Temperature Pointer Locations: ********" << endl;
			cerr << "Number of rows in unsolved:" << (*datalibrary_->unsolved_matrix_dyn_).nrows() << endl;
			cerr << "Number of rows in unsolved, removing num_locktemp_rows_:" << (*datalibrary_->unsolved_matrix_dyn_).nrows()-datalibrary_->num_locktemp_rows_ << endl;
			cerr << "Number of elements in vector_layer:" << vector_layer.size() << endl;
			cerr << "Number of elements in the solution temperature column:" << datalibrary_->temperature_matrix_size_ << endl;
			Matrix_itor i,j;
			error=false;
			//make sure to skip the lock-temp equations at the bottom of temperature column, these are not mapped to any of the model units 
			for (int itor=0; itor < (*datalibrary_->unsolved_matrix_dyn_).nrows(); itor++) {
				if(!EQ(datalibrary_->temperature_matrix_[itor],*get_dyn_array(vector_layer[itor])[vector_x[itor]][vector_y[itor]].get_temperature())){
					cerr  << tempVector[itor+1] << " " << "L" << vector_layer[itor] << "[" << vector_x[itor] << "][" << vector_y[itor] << "].temperature() != " << *get_dyn_array(vector_layer[itor])[vector_x[itor]][vector_y[itor]].get_temperature()  << "]" << endl;
					error=true;
				}
				else
					cerr  << tempVector[itor+1] << " " << "L" << vector_layer[itor] << "[" << vector_x[itor] << "][" << vector_y[itor] << "].temperature() (" << datalibrary_->temperature_matrix_[itor] << ") != " << *get_dyn_array(vector_layer[itor])[vector_x[itor]][vector_y[itor]].get_temperature()  << "]" << endl;
			}
			if(!error)
				cerr << "******** Temperature Pointer Locations are Correct! ********" << endl;				
			else
				cerr << "******** Temperature Pointer Locations are NOT Correct! ********" << endl;		
			
			error=false;
			
			cerr << "******** Checking For Proper Equation Generation (LHS and RHS must be equal): ********" << endl;
			int itor=0;
			double lhs_total=0.0;
			for (j=0; j < (*datalibrary_->unsolved_matrix_dyn_).nrows(); ++j) {
				
				lhs_total=0.0;
				for (i=0; i < (*datalibrary_->unsolved_matrix_dyn_).ncols(); ++i) 
					lhs_total+=(*datalibrary_->unsolved_matrix_dyn_)(j,i);
				if(!EQ(datalibrary_->temperature_matrix_[itor],lhs_total)){
					cerr  << tempVector[itor+1] << " " << "NOT EQUAL[lhs,rhs]:" << "[" << lhs_total << "]" << "[" << datalibrary_->temperature_matrix_[itor] << "] dif:[" << fabs(lhs_total-datalibrary_->temperature_matrix_[itor]) << "]" << endl;
					error=true;
				}
				itor++;
			}
			
			if(!error)
				cerr << "******** LHS and RHS of Equations are Equal! ********" << endl;						
			else
				cerr << "******** LHS and RHS of Equations are NOT Equal! ********" << endl;
			
			cerr << "******** Checking For Steady-State Solution (temperature should be 1 Celcius for every unit): ********" << endl;
			cerr << "********		MAKE SURE THAT LOCK-TEMP IS DISABLED FOR ALL ROWS (OTHERWISE THIS WILL FAIL) ********" << endl;
			//set heat generation to zero in all layers and set the layer temperatures to one
			for(uint32_t j=0;j<datalibrary_->all_layers_info_.size();j++){
				set_temperature_layer(j,1.0);		//set the temperature to 25 celcius
				if(datalibrary_->all_layers_info_[j]->chip_floorplan_!=NULL){
					for(uint32_t i=0;i<chip_flp_count(j);i++) {
						set_power_flp(i, j,  0.0); //set the power generation to zero
					}
				}
			}			
			
			
			//recompute the material properties
			recompute_material_properties();
			
			
			
			//recompute the matrix
			y_itor_global=0;
			for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
				if(datalibrary_->all_layers_info_[layer]->layer_used_){
					for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
						for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
							if(!get_dyn_array(layer)[x_itor][y_itor].defined_)
								continue;
							
							get_dyn_array(layer)[x_itor][y_itor].calculate_terms(datalibrary_->config_data_->default_time_increment_, false, false);
							y_itor_global++;
						}
				}	
					
					//create SuperLUMatrix
					createSuperLUMatrix();
					//solve the matrix, generating new temperatures
					datalibrary_->timestep_=.00001;
					//advance the model by timestep
					datalibrary_->time_+=.00001;
					solve_matrix_superlu();		//run the model
					
					error=false;
					for(int i=0;i<(*datalibrary_->unsolved_matrix_dyn_).nrows();i++){
						
						if(!EQ(datalibrary_->temperature_matrix_[i],1.0)){
							cerr  << tempVector[i+1] << " " << setw(4) << "NOT EQUAL:" << "[" << datalibrary_->temperature_matrix_[i] << "]" << endl;
							error=true;
						}
					}
					
					if(!error)
						cerr << "******** Steady-State Solution is Correct (temperatures are all 1 celcius) ! ********" << endl;						
					else
						cerr << "******** Steady-State Solution is NOT Correct (temperatures are all 1 celcius) ! ********" << endl;						
}

//by default, the time increment config_data_->default_time_increment_ is used
void sesctherm3Dmodel::compute(bool recompute_material_properties){
	compute(datalibrary_->config_data_->default_time_increment_, recompute_material_properties);
}

//make a call to the solver, incrementing the current time by the time increment
//if the sampler is enabled, then accumulate data for each sample 
void sesctherm3Dmodel::compute(double timestep, bool recompute_material_properties) {
	
	//if samples are disabled, then just run normally
	if(datalibrary_->config_data_->enable_samples_==false){
#ifdef _SESCTHERM_DEBUG
		cerr << "Sampling Disabled" << endl;
#endif
		
		run_model(timestep, datalibrary_->accumulate_rc_, datalibrary_->use_rc_,  recompute_material_properties);			//run the model
		
	}else{
		//samples are enabled
#ifdef _SESCTHERM_DEBUG
		//	cerr << "Sampling Enabled" << endl;
#endif
		run_sampler(timestep, datalibrary_->use_rc_, recompute_material_properties);
	}
}

void sesctherm3Dmodel::run_sampler(double timestep, bool use_rc, bool recompute_material_properties){
	//if the sample_start_time has arrived, and we aren't beyond the sample_end_time, perform sampling 
	//Note: If the sample duration is so long that it runs past the sample_end_time, no sample will be stored.
	//		This is done to ensure that the samples are of equal length. If the sample_end_time_ were also to
	//		perform a sample computation, the last sample could be less than the length of the sample duration.
	if(GE(datalibrary_->time_+timestep, datalibrary_->config_data_->sample_start_time_) && LT(datalibrary_->time_+timestep, datalibrary_->config_data_->sample_end_time_)){
		
		//If we haven't reached the start of the sample, but the timestep is large enough to
		//pass the start point of the sample, then run the model to the beginning of the sample
		if(LT(datalibrary_->time_, datalibrary_->config_data_->sample_start_time_)){
			run_model(datalibrary_->config_data_->sample_start_time_-datalibrary_->time_, datalibrary_->accumulate_rc_, use_rc,recompute_material_properties);
			timestep=timestep-datalibrary_->timestep_;	//set the new timestep to be equal to the remaining time after the sample_start_time
		}
		
		//if we haven't passed the end of the current sample,then accumulate data
		//if we reached the end of the sample, then also compute the averages and generate images
		if(LE(datalibrary_->time_+timestep,datalibrary_->cur_sample_end_time_)){			
			run_model(timestep, datalibrary_->accumulate_rc_, use_rc,recompute_material_properties);
			
			//accumulate data*timestep values in each of the layers
			for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++)
				datalibrary_->all_layers_info_[i]->accumulate_data_for_sample();
			
			//if we've reached the end of one sample, compute averages/max/min across sample and generate images
			if(EQ(datalibrary_->time_+timestep,datalibrary_->cur_sample_end_time_)){
				for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++){
					datalibrary_->all_layers_info_[i]->compute_sample();
					
				}
				datalibrary_->graphics_->print_graphics();		//generate and output all the graphics data
				datalibrary_->cur_sample_end_time_+=datalibrary_->config_data_->sample_duration_;
			}
		}
		//if the timestep is so large that is runs across the boundary of the sample,
		//then run to one (or more) sample endpoints, storing data for each and generating graphics as necessary
		else{
			double run_end_time=datalibrary_->time_+timestep;
			//run the model multiple times, collecting data for each sample until we reach the end of the run
			while(LE(datalibrary_->cur_sample_end_time_, run_end_time) &&
				  LT(datalibrary_->time_, datalibrary_->config_data_->sample_end_time_)){
				//timestep will be the time to the end of the current sample (or the sample_end_time)
				//move on to the next sample
				datalibrary_->cur_sample_end_time_+=datalibrary_->config_data_->sample_duration_;
				//advance the model to the end of the sample
				run_model(datalibrary_->cur_sample_end_time_-datalibrary_->time_, datalibrary_->accumulate_rc_, use_rc,recompute_material_properties);
				
				
				//accumulate temperature*timestep values in each of the layers
				for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++)
					datalibrary_->all_layers_info_[i]->accumulate_data_for_sample();
				
				for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++){
					datalibrary_->all_layers_info_[i]->compute_sample();
					
				}
				
				
				datalibrary_->graphics_->print_graphics();		//generate and output all the graphics data
				
			}
		}
	}
	else{
		run_model(timestep, datalibrary_->accumulate_rc_, use_rc,recompute_material_properties);
	}	
}


//just run the model without handling samples
void sesctherm3Dmodel::run_model(double timestep, bool accumulate_rc, bool use_rc, bool recompute_material_properties){
	
	datalibrary_->time_+=timestep;
	
	if(timestep!=datalibrary_->timestep_) {
		datalibrary_->timestep_=timestep;
		//Note: false here means that we should recompute
		//Whenever the timestep changes we need to recompute
		compute_model_units(false, timestep, accumulate_rc, use_rc);	//compute all the new temperature data given the new energy data
	}else{
		//variable inverted for desired behavior (true=recompute as input value is "calc_solution_only"--should be false for recomputation)
		compute_model_units(!recompute_material_properties, timestep, accumulate_rc, use_rc);					//compute all the new temperature data given the new energy data
	}
	
	solve_matrix_superlu();
}

//store power data to the functional unit with the given name
void sesctherm3Dmodel::set_power_flp(int flp_pos, int32_t layer, double power) {
	if(datalibrary_->all_layers_info_[layer]->chip_floorplan_ != NULL){
		datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_[flp_pos].set_power(power,datalibrary_->all_layers_info_[layer]->thickness_);
	}
	else{
		cerr << "FATAL: sesctherm3Dmodel::store_power_flp => layer [" << layer << "] has undefined chip_floorplan" << endl;
		exit(1);
	}
}

void sesctherm3Dmodel::set_power_layer(int layer, vector<double>& power_map){
	
	if(datalibrary_->all_layers_info_[layer]->chip_floorplan_ == NULL){
		cerr << "FATAL: sesctherm3Dmodel::set_power_layer => source layer [" << layer << "] has undefined chip_floorplan" << endl;
		exit(1);
	}
	
	I(power_map.size() == datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_.size());
	
	for(int itr1 = 0;itr1<(int)datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_.size();itr1++){
		datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_[itr1].set_power(power_map[itr1],datalibrary_->all_layers_info_[layer]->thickness_ );	
	}
}

//store max power data to the functional unit with the given name
void sesctherm3Dmodel::set_max_power_flp(int flp_pos, int32_t layer, double power) {
	if(datalibrary_->all_layers_info_[layer]->chip_floorplan_ != NULL){
	    I(flp_pos > 0 && flp_pos < (int)datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_.size());
		datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_[flp_pos].set_max_power(power);
	}
	else{
		cerr << "FATAL: sesctherm3Dmodel::set_max_power_flp => layer [" << layer << "] has undefined chip_floorplan" << endl;
		exit(1);
	}
}


//return the temperature map
vector<double> sesctherm3Dmodel::get_temperature_layer(int flp_layer, int32_t source_layer) {
	return (datalibrary_->all_layers_info_[source_layer]->compute_average_temps(flp_layer));
}

double sesctherm3Dmodel::get_temperature_flp(int flp_pos, int32_t flp_layer, int32_t source_layer){
	return(datalibrary_->all_layers_info_[source_layer]->compute_average_temps(flp_layer)[flp_pos]);
}

vector<double> sesctherm3Dmodel::get_power_layer(int flp_layer){
	return(datalibrary_->all_layers_info_[flp_layer]->compute_average_powers(flp_layer));
}	

double sesctherm3Dmodel::get_power_flp(int flp_layer, int32_t flp_pos){
	return(datalibrary_->all_layers_info_[flp_layer]->compute_average_powers(flp_layer)[flp_pos]);
}

double sesctherm3Dmodel::get_time() {
	return(datalibrary_->time_);
}

string sesctherm3Dmodel::get_chip_flp_name(int flp_pos, int32_t layer) {
	
	if(datalibrary_->all_layers_info_[layer]->chip_floorplan_ != NULL) {
		I(flp_pos > 0 && flp_pos < (int)datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_.size());
		return(datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_[flp_pos].name_);
	}
	
	cerr << "FATAL: sesctherm3Dmodel::get_chip_flp_name => layer [" << layer << "] has undefined chip_floorplan" << endl;
	exit(1);
}


void sesctherm3Dmodel::set_temperature_flp(int flp_pos, int32_t flp_layer, int32_t source_layer, double temp){
	//find the floorplan unit
	chip_flp_unit& flp_unit=datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[flp_pos];
	dynamic_array<model_unit>& layer_dyn=(*datalibrary_->all_layers_info_[source_layer]->floorplan_layer_dyn_);
	//find all the model units that correspond to this floorplan unit
	if (flp_unit.located_units_[source_layer].empty() ) {
		model_unit::locate_model_units(source_layer,
									   flp_unit.located_units_[source_layer],
									   flp_unit.leftx_,
									   flp_unit.bottomy_,
									   flp_unit.leftx_+flp_unit.width_,
									   flp_unit.bottomy_+flp_unit.height_,
									   datalibrary_);
	}
	//set the temperature to each model unit that cooresponds to the particular floorplan unit
	//Note: no need to check if the temperature_ pointer has been set (locate_model_units will only return non-zero pointers)
	for(int j=0;j<(int)flp_unit.located_units_[source_layer].size();j++){
		*layer_dyn[flp_unit.located_units_[source_layer][j]->x_coord_][flp_unit.located_units_[source_layer][j]->y_coord_].get_temperature()=temp;
		*layer_dyn[flp_unit.located_units_[source_layer][j]->x_coord_][flp_unit.located_units_[source_layer][j]->y_coord_].get_previous_temperature()=temp;
	}	
	
	//recompute the material properties as a function of temperature
	recompute_material_properties(source_layer);
}

void sesctherm3Dmodel::set_temperature_layer(int flp_layer, int32_t source_layer, vector<double>& temperature_map) {
	
	if(datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_ == NULL){
		cerr << "FATAL: sesctherm3Dmodel::set_temp_map => flp layer [" << flp_layer << "] has undefined chip_floorplan" << endl;
		exit(1);
	}
	dynamic_array<model_unit>& layer_dyn=(*datalibrary_->all_layers_info_[source_layer]->floorplan_layer_dyn_);
	
	for(int itr1 = 0;itr1<(int)temperature_map.size();itr1++){
		//find the floorplan unit
		chip_flp_unit& flp_unit=datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[itr1];
		
		//find all the model units that correspond to this floorplan unit
		if (flp_unit.located_units_[source_layer].empty() ) {
			model_unit::locate_model_units(source_layer,
										   flp_unit.located_units_[source_layer],
										   flp_unit.leftx_,
										   flp_unit.bottomy_,
										   flp_unit.leftx_+flp_unit.width_,
										   flp_unit.bottomy_+flp_unit.height_,
										   datalibrary_);
		}
		//set the temperatures in each model unit to correspond to the value in the temperature map
		//Note: no need to check if the temperature_ pointer has been set (locate_model_units will only return non-zero pointers)
		for(int j=0;j<(int)flp_unit.located_units_[source_layer].size();j++) {
			*layer_dyn[flp_unit.located_units_[source_layer][j]->x_coord_][flp_unit.located_units_[source_layer][j]->y_coord_].get_temperature()=temperature_map[itr1];
			*layer_dyn[flp_unit.located_units_[source_layer][j]->x_coord_][flp_unit.located_units_[source_layer][j]->y_coord_].get_previous_temperature()=temperature_map[itr1];
		}	
	}
	
	//recompute the material properties as a function of temperature
	recompute_material_properties(source_layer);
}

void sesctherm3Dmodel::set_temperature_layer(int source_layer, double temperature){
	sesctherm3Dlayerinfo& layer=*datalibrary_->all_layers_info_[source_layer];
	dynamic_array<model_unit>& layer_dyn=(*datalibrary_->all_layers_info_[source_layer]->floorplan_layer_dyn_);
	//find all the model units within this layer
	if (layer.located_units_[source_layer].empty() ) {
		model_unit::locate_model_units(source_layer,
									   layer.located_units_[source_layer],
									   layer.leftx_,
									   layer.bottomy_,
									   layer.rightx_,
									   layer.topy_,
									   datalibrary_);
	}
	//set the temperatures in each model unit to correspond to the value in the temperature map
	//Note: no need to check if the temperature_ pointer has been set (locate_model_units will only return non-zero pointers)
	for(int j=0;j<(int)layer.located_units_[source_layer].size();j++){
		*layer_dyn[layer.located_units_[source_layer][j]->x_coord_][layer.located_units_[source_layer][j]->y_coord_].get_temperature()=temperature;
		*layer_dyn[layer.located_units_[source_layer][j]->x_coord_][layer.located_units_[source_layer][j]->y_coord_].get_previous_temperature()=temperature;
		layer_dyn[layer.located_units_[source_layer][j]->x_coord_][layer.located_units_[source_layer][j]->y_coord_].lock_temp_=temperature;
	}	
	//recompute the material properties as a function of temperature
	recompute_material_properties(source_layer);
}


uint32_t sesctherm3Dmodel::chip_flp_count(int layer){
	if(datalibrary_->all_layers_info_[layer]->chip_floorplan_ == NULL){
		cerr << "FATAL: sesctherm3Dmodel::chip_flp_count => layer [" << layer << "] has undefined chip_floorplan" << endl;
		exit(1);
	}
	return(datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_units_.size());
}


void sesctherm3Dmodel::set_ambient_temp(double temp) {
	datalibrary_->config_data_->ambient_temp_=temp;
}

void sesctherm3Dmodel::set_initial_temp(double temp) {
	datalibrary_->config_data_->init_temp_=temp;
}


//FIXME: this data should be dumped to an [all temperatures] section of the outfile
void sesctherm3Dmodel::dump_temp_data() {
	I(0);
}

//FIXME: this function needs to be written
//It will calculate a weighted average of all of the model units that belong to each flp
//It will then dump the temperature data of each flp in the order that the labels were printed
//This will be dumped to an [average temps] section of the outfile
void sesctherm3Dmodel::dump_flp_temp_data(){
    I(0);
}

//FIXME: this function needs to be written
//It will print the labels of the flp in lab-deliminated fashion at the top of the outfile
void sesctherm3Dmodel::print_flp_temp_labels() {
    I(0);
}

//FIXME: this function needs to be written
//It will create an running average of the temperature for each of the model_units
//This will be a very fine-grained method of warmup as the entire chip will be warmed up
void sesctherm3Dmodel::warmup_chip() {
    I(0);
}

//This function will take the data that was written in the dumpfile in either
// the [average temps] section or the [all temps] section and create a 3d graph of the data
void sesctherm3Dmodel::dump_3d_graph() {
    I(0);
}



void sesctherm3Dmodel::copy_solution_to_floorplan_superlu(SuperMatrix& A) {
	DNformat* Astore= (DNformat*)A.Store;
	
	SUElement_t       *dp;
	int y_itor_global=0;
	dp = (SUElement_t *) Astore->nzval;
	
	//swap temperature matrix and previous temperature matrix
	//this effectively copies the new solution to the previous_temperature_matrix_
	
	/*cerr << "previous matrix:" << datalibrary_->previous_temperature_matrix_ << endl;
	cerr << "new matrix:" << datalibrary_->temperature_matrix_ << endl;
	cerr << "dp:" << dp;
	
	SUElement_t* temp_matrix=datalibrary_->previous_temperature_matrix_;
	datalibrary_->previous_temperature_matrix_=(SUElement_t*)Astore->nzval;
	datalibrary_->temperature_matrix_=temp_matrix;
	Astore->nzval=temp_matrix;
	*/
	
	memcpy(datalibrary_->temperature_matrix_,dp,datalibrary_->unsolved_matrix_dyn_->nrows()*sizeof(SUElement_t));
	//	datalibrary_->temperature_matrix_=dp;
	
	
	/*	
		for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
	 if(datalibrary_->all_layers_info_[layer]->layer_used_){
		 for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
			 for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
				 if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
					 continue;   //skip if unit is not defined
				 if(y_itor_global >= A.nrow){
					 cerr << "sesctherm3Dmodel::copy_solution_to_floorplan_superlu -> y_itor_global beyond A->nrows" << endl;
					 cerr << "		(attempting to access element of SuperMatrix B that is beyond the max element)" << endl;
					 exit(1);
				 }
				 // get_dyn_array(layer)[x_itor][y_itor].get_temperature()=dp[y_itor_global];
				 // get_dyn_array(layer)[x_itor][y_itor].get_temperature()=dp[y_itor_global];
				 cerr << dp[y_itor_global] << endl;
				 y_itor_global++;
			 }	
	 }
	 */ 
	
}





void sesctherm3Dmodel::solve_matrix_superlu(){
	int      m, n;
	trans_t  trans;
	
	trans             = NOTRANS;
	
	m=datalibrary_->unsolved_matrix_dyn_->nrows();
	n=datalibrary_->unsolved_matrix_dyn_->ncols();
	
	
	//	datalibrary_->rhs_superlu_=datalibrary_->temperature_matrix_;
	
	
	//	if(datalibrary_->rhs_size_superlu_==-1){
	//		datalibrary_->rhs_size_superlu_=datalibrary_->unsolved_matrix_dyn_->nrows();
	//		datalibrary_->rhs_superlu_=new SUElement_t[datalibrary_->rhs_size_superlu_];
	//		dCreate_Dense_Matrix(&datalibrary_->B_superlu_, m, 1, datalibrary_->rhs_superlu_, m, SLU_DN, SLU_D, SLU_GE);
	//	}
	//	
	//	memcpy(datalibrary_->rhs_superlu_,datalibrary_->temperature_matrix_,datalibrary_->unsolved_matrix_dyn_->nrows()*sizeof(SUElement_t));	
	
	
	//datalibrary_->rhs_superlu_=datalibrary_->temperature_matrix_;
	if(datalibrary_->rhs_size_superlu_==-1) {
		dCreate_Dense_Matrix(&datalibrary_->B_superlu_, m, 1, datalibrary_->temperature_matrix_, m, SLU_DN, SLU_D, SLU_GE);
	}
	
	((DNformat*)(datalibrary_->B_superlu_.Store))->nzval=datalibrary_->temperature_matrix_;
	
	/* ------------------------------------------------------------
		Solve the system A*X=B, overwriting B with X.
		------------------------------------------------------------*/
	//    dgstrs(trans, &datalibrary_->L_superlu_, &datalibrary_->U_superlu_, datalibrary_->permr_superlu_, datalibrary_->permc_superlu_, &B, &datalibrary_->Gstat_superlu_, &datalibrary_->info_superlu_);
	
	dgstrs(trans, &datalibrary_->L_superlu_, &datalibrary_->U_superlu_, datalibrary_->permr_superlu_, datalibrary_->permc_superlu_
		   , &datalibrary_->B_superlu_
		   , &datalibrary_->Gstat_superlu_, &datalibrary_->info_superlu_);	
	//copy_solution_to_floorplan_superlu(datalibrary_->B_superlu_);
	
	//memcpy(((DNformat*)(datalibrary_->B_superlu_.Store))->nzval,datalibrary_->temperature_matrix_, datalibrary_->unsolved_matrix_dyn_->nrows());
	
	//delete [] datalibrary_->rhs_superlu_;
	
	//	Destroy_SuperMatrix_Store(&datalibrary_->B_superlu_);
}


//Note: this should ONLY be called once (to generate model matrix, not the solution matrices)
void sesctherm3Dmodel::createSuperLUMatrix(){
	
#ifdef USE_MP
	pdgstrf_options_t options;
	pxgstrf_shared_t pxgstrf_shared;
	pdgstrf_threadarg_t *pdgstrf_threadarg;
#else
	superlu_options_t options;
#endif
	
	int         nprocs;
	fact_t      fact;
	trans_t     trans;
	yes_no_t    refact, usepr;
	double      u, drop_tol;
	SUElement_t      *a;
	int         *asub, *xa;
	void        *work;
	int			lwork, nrhs, ldx; 
	int         n,m,nnz, permc_spec, panel_size, relax;
	int         i, firstfact;
	SUElement_t      *rhsb, *xact;
	
	flops_t     flopcnt;
	
	//convert dense matrix format into Harwell-Boeing for SuperLU (old method, no longer necessary)
	/*	
		
	 for (int i=0;i<datalibrary_->unsolved_matrix_dyn_->nrows();i++)
	 for (int j=0;j<datalibrary_->unsolved_matrix_dyn_->ncols();j++)
	 if (!EQ((*datalibrary_->unsolved_matrix_dyn_)(j,i),0.0)) num_non_zero++;
#ifdef _SESCTHERM_DEBUG
	 cout << num_non_zero << " is the number of non-zero values in the matrix for LU decomposition" << endl;
#endif	
	 cout << "num_non_zero by method 1:" << num_non_zero << "num_non_zero by method 2:" << num_non_zero_test << endl;
	 
	 SUElement_t * non_zero_vals= new SUElement_t[num_non_zero];
	 int32_t * row_indices= new int[num_non_zero];
	 int32_t * col_begin_indices = new int[datalibrary_->unsolved_matrix_dyn_->ncols()+1]; 
	 
	 
	 int32_t k=0;
	 int32_t j=0;
	 for (j=0;j<datalibrary_->unsolved_matrix_dyn_->ncols();j++)
	 {
		 col_begin_indices[j] = k;
		 for (int i=0;i<datalibrary_->unsolved_matrix_dyn_->nrows();i++)
			 if (!EQ((*datalibrary_->unsolved_matrix_dyn_)(j,i),0.0))
			 {
				 row_indices[k] = i;
				 non_zero_vals[k] = (*datalibrary_->unsolved_matrix_dyn_)(j,i);
				 k++;
			 }
	 }
	 col_begin_indices[j]=k;
	 */
	
    
	//for (int i=0; i<m+1; i++)
	//	col_begin_indices[i] += 1;
	
    //for (int i=0; i<num_non_zero; i++)
	//	row_indices[i] += 1;
	
	m=datalibrary_->unsolved_matrix_dyn_->nrows();
	n=datalibrary_->unsolved_matrix_dyn_->ncols();
	nnz=datalibrary_->unsolved_matrix_dyn_->nnz();		
	
	if(datalibrary_->non_zero_vals_==NULL){
		datalibrary_->non_zero_vals_= new SUElement_t[nnz];
		datalibrary_->row_indices_= new int[nnz];
		datalibrary_->col_begin_indices_ = new int[datalibrary_->unsolved_matrix_dyn_->ncols()+1]; 
	}
	
	SUElement_t * non_zero_vals_storage=datalibrary_->unsolved_matrix_dyn_->get_val();	
	
	int * row_indices_storage=datalibrary_->unsolved_matrix_dyn_->get_ind();
	
	int * col_begin_indices_storage=datalibrary_->unsolved_matrix_dyn_->get_ptr();
	
	memcpy(datalibrary_->non_zero_vals_,non_zero_vals_storage,nnz*sizeof(SUElement_t));
	memcpy(datalibrary_->row_indices_,row_indices_storage,nnz*sizeof(int));
	memcpy(datalibrary_->col_begin_indices_,col_begin_indices_storage,  (datalibrary_->unsolved_matrix_dyn_->ncols()+1)*sizeof(int));
	
	/*
	 for(int i=0;i<num_non_zero;i++){
		 cout <<  "[" << non_zero_vals[i] << "][" << non_zero_vals_test[i] << "]" << endl;
		 cout <<  "[" << row_indices[i] << "][" << row_indices_test[i] << "]" << endl;
		 cout <<  "[" << col_begin_indices[i] << "][" << col_begin_indices_test[i] << "]" << endl;			
	 }
	 exit(1);
	 */
	
	dCreate_CompCol_Matrix(&datalibrary_->A_, m, n, nnz, datalibrary_->non_zero_vals_, 
						   datalibrary_->row_indices_, datalibrary_->col_begin_indices_, 
						   SLU_NC, SLU_D, SLU_GE);
	
	//void parse_command_line();
	
	/* Default parameters to control factorization. */
	nprocs = datalibrary_->config_data_->num_processors_;
	fact  = DOFACT;
	trans = NOTRANS;
	panel_size = sp_ienv(1);
	relax = sp_ienv(2);
	u     = 1.0;
	usepr = NO;
	drop_tol = 0.0;
	work = NULL;
	lwork = 0;
	nrhs  = 1;
	
	/********************************
		* THE FIRST TIME FACTORIZATION *
		********************************/
	
	/* ------------------------------------------------------------
		Allocate storage and initialize statistics variables. 
		------------------------------------------------------------*/
#ifdef USE_MP
	StatAlloc(n, nprocs, panel_size, relax, &datalibrary_->Gstat_superlu_);
	StatInit(n, nprocs, &datalibrary_->Gstat_superlu_);
#else
	StatInit(&datalibrary_->Gstat_superlu_);
#endif
	
	/* ------------------------------------------------------------
		Get column permutation vector perm_c[], according to permc_spec:
		permc_spec = 0: natural ordering 
		permc_spec = 1: minimum degree ordering on structure of A'*A
		permc_spec = 2: minimum degree ordering on structure of A'+A
		permc_spec = 3: approximate minimum degree for unsymmetric matrices
		------------------------------------------------------------*/ 	
#ifdef USE_MP
	permc_spec = 1;
#else
	permc_spec = 2; // 2 only works with non-parallel implementatio and it is 1.8times faster
#endif
	if(datalibrary_->permc_superlu_ == NULL){
		datalibrary_->permc_superlu_ = new int[n];
		datalibrary_->permr_superlu_ = new int[m];
		datalibrary_->etree_superlu_ = new int[m];
	}
	get_perm_c(permc_spec, &datalibrary_->A_, datalibrary_->permc_superlu_);
	
	/* ------------------------------------------------------------
		Initialize the option structure options using the
		user-input parameters;
	Apply perm_c to the columns of original A to form AC.
		
		------------------------------------------------------------*/
	refact= NO;
	SuperMatrix AC;
	
#ifdef USE_MP
	pdgstrf_init(nprocs, refact, panel_size, relax,
				 u, usepr, drop_tol, datalibrary_->permc_superlu_, datalibrary_->permr_superlu_,
				 work, lwork, &datalibrary_->A_, &AC, &options, &datalibrary_->Gstat_superlu_);
#else
	set_default_options(&options); 
#endif
	
	/* ------------------------------------------------------------
		Compute the LU factorization of A.
		The following routine will create nprocs threads.
		------------------------------------------------------------*/
	
#ifdef USE_MP
	pdgstrf(&options, &AC, datalibrary_->permr_superlu_, &datalibrary_->L_superlu_, &datalibrary_->U_superlu_, &datalibrary_->Gstat_superlu_, &datalibrary_->info_superlu_);
	
	flopcnt = 0;
	
	for (i = 0; i < nprocs; ++i) flopcnt += datalibrary_->Gstat_superlu_.procstat[i].fcops;
	datalibrary_->Gstat_superlu_.ops[FACT] = flopcnt;
#else
	sp_preorder(&options, &datalibrary_->A_, datalibrary_->permc_superlu_, datalibrary_->etree_superlu_, &AC); 
	
	dgstrf(&options, &AC, drop_tol,
		   relax, panel_size, datalibrary_->etree_superlu_, 0, 0,
		   datalibrary_->permc_superlu_, datalibrary_->permr_superlu_, &datalibrary_->L_superlu_, &datalibrary_->U_superlu_,
		   &datalibrary_->Gstat_superlu_, &datalibrary_->info_superlu_);
#endif
	
}




void sesctherm3Dmodel::fill_solution_matrix_superlu(SUElement_t* solution_matrix) {	
	/*    //Note: need to create last entry (where last entry is equal to T_ambient, as T_inf must be equal to T_ambient
    int32_t y_itor_global=0;
    for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
	if(datalibrary_->all_layers_info_[layer]->layer_used_){
		for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
			for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
				if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
					continue;   //skip if unit is not defined
				solution_matrix[y_itor_global]=get_dyn_array(layer)[x_itor][y_itor].get_temperature();
				y_itor_global++;
			}
	}
	
	//t_INF is always equal to ambient! (infinite reservoir of heat)
	solution_matrix[datalibrary_->unsolved_matrix_dyn_->nrows()-1]=datalibrary_->config_data_->ambient_temp_;
	*/
}





//FIXME:  I'm currently designating unused nodes to the vacant regions in between the fins! These nodes should be removed (save memory and calculation time)
//return the index in the unsolved_matrix_dyn row given the layer, x_coord and y_coord of the model unit index
int sesctherm3Dmodel::find_unsolved_matrix_row_index(int layer, int32_t x_coord, int32_t y_coord) {
    uint32_t itor_x=0;
    uint32_t itor_y=0;
    int32_t virtual_layer_row_index_=0;
    int32_t heat_sink_fins_unused_columns=0;
    int32_t heat_sink_fins_index=0;
	int index=0;
	int offset=0;
	
	I(layer>=0 && layer < (int)datalibrary_->all_layers_info_.size());
	
	
	//find the column offset to the column entries that correspond to the specified layer
	//that is offet by the number of model units in layers 0 to (layer-1)
	for(int i=0;i<layer;i++){
		index+=datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->ncols()*datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->nrows()-datalibrary_->all_layers_info_[i]->unused_dyn_count_;
	}
	
	//now find the offset within the layer we are in (ncols*y_coord+x_coord)
	
	//if we are in the ucool_layer/heat sink fins layer there will be an uneven number of unused units in each row, we need to handle this
	//by using an iterative search
	if(datalibrary_->all_layers_info_[layer]->type_==UCOOL_LAYER || datalibrary_->all_layers_info_[layer]->type_==HEAT_SINK_FINS_LAYER){
		//we need an iterative search as there will be a variable number of unused model units on the various rows
		for (itor_y=0;itor_y<=datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->nrows();itor_y++) {
			for (itor_x=0;itor_x<datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->ncols();itor_x++) {
				if((int)itor_y==y_coord && (int)itor_x==x_coord){
					return(index);
				}
				if ((*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[itor_x][itor_y].defined_==true)
					index++;
			}
		}
		I(0);
	}
	I(index+(int)(datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->ncols())*y_coord+x_coord >= 0);
	//SUElement_t tmp=index+datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->ncols()*y_coord+x_coord;
	return(index+datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->ncols()*y_coord+x_coord);
	
}



void sesctherm3Dmodel::find_unused_model_units() {
    //find the number of number of unused columns in the heat_sink_fins_floorplan_dyn_
    uint32_t itor_x=0;
    uint32_t itor_y=0;
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++)
		datalibrary_->all_layers_info_[i]->unused_dyn_count_=0;
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		if(datalibrary_->all_layers_info_[i]->type_==HEAT_SINK_FINS_LAYER || datalibrary_->all_layers_info_[i]->type_==UCOOL_LAYER && datalibrary_->all_layers_info_[i]->layer_used_){
			for (itor_x=0;itor_x<datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->ncols();itor_x++) {
				for (itor_y=0;itor_y<datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->nrows();itor_y++) {
					if ((*datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_)[itor_x][itor_y].defined_==false)
						datalibrary_->all_layers_info_[i]->unused_dyn_count_++;
				}
			}
		}
	}
	
}


void sesctherm3Dmodel::store_pointer(int x_coord, int32_t y_coord, SUElement_t*& pointer_location, bool set_pointer) {
	if(set_pointer){
		pointer_location = &(*(*datalibrary_->unsolved_matrix_dyn_)(y_coord, x_coord).iter);	//set the pointer to that location
																								//cerr << setw(4) << (*datalibrary_->unsolved_matrix_dyn_)(x_coord,y_coord) << "[" << x_coord << "," << y_coord << "]" << endl;
	}else{
		(*datalibrary_->unsolved_matrix_dyn_)(y_coord, x_coord)=-1.0;	//allocate a space in the sparse matrix
	}
}

//this will set the pointers of the model unit's coefficient pointer to point to the location in the unsolved matrix
void sesctherm3Dmodel::initialize_unsolved_matrix_row(int y_itor_global, int32_t layer, int32_t x_itor, int32_t y_itor, bool set_pointer) {
	//Check: ensure that if two adjacent layers are of same dimension (in either x or y direction), that the offset is zero
	
	int layer_above=MIN(layer+1,(int)datalibrary_->all_layers_info_.size()-1);
	int layer_below=MAX(layer-1,0);
	
	dynamic_array<model_unit>& temp_floorplan_dyn=get_dyn_array(layer);
	// Tm,n,o
	store_pointer(find_unsolved_matrix_row_index(layer,x_itor,y_itor), y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno, set_pointer);
	// Tm-1,n,o
	if(!EQ(temp_floorplan_dyn[x_itor][y_itor].x1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].x1_,0.0))
		store_pointer(find_unsolved_matrix_row_index(layer,x_itor-1,y_itor),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_m_M1_no, set_pointer);
	// Tm+1,n,o
	if(!EQ(temp_floorplan_dyn[x_itor][y_itor].x3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].x3_,0.0))
		store_pointer(find_unsolved_matrix_row_index(layer,x_itor+1,y_itor), y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_m_P1_no, set_pointer);
	// Tm,n+1,o
	if (!EQ(temp_floorplan_dyn[x_itor][y_itor].y3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].y3_,0.0)) {
		//skip the UCOOL layer
		if(datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER){
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+2,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+2,datalibrary_);
			
			store_pointer(find_unsolved_matrix_row_index(layer+2, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_P1_o, set_pointer);
		}else{
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			
			
			store_pointer(find_unsolved_matrix_row_index(layer+1, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_P1_o, set_pointer);
		}
		
	}
	// Tm,n-1,o
	if (!EQ(temp_floorplan_dyn[x_itor][y_itor].y1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].y1_,0.0)) {
		if(layer==0){
			cerr << "FATAL: sesctherm3Dmodel::create_unsolved_matrix_row: Tm,n-1,o MUST be 0 on Layer"<< layer << "(and it's not!)" << endl;
			exit(1);
		}
		//skip the UCOOL layer
		if(datalibrary_->all_layers_info_[layer-1]->type_==UCOOL_LAYER){
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-2,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-2,datalibrary_);
			
			store_pointer(find_unsolved_matrix_row_index(layer-2, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_M1_o, set_pointer);
		}else{
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			
			
			store_pointer(find_unsolved_matrix_row_index(layer-1, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_M1_o, set_pointer);
		}	
		
	}
	// Tm,n,o-1
	if (!EQ(temp_floorplan_dyn[x_itor][y_itor].z1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].z1_,0.0)) 
		store_pointer(find_unsolved_matrix_row_index(layer,x_itor,y_itor-1),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno_M1, set_pointer);
	// Tm,n,o+1
	if (!EQ(temp_floorplan_dyn[x_itor][y_itor].z3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].z3_,0.0)) 
		store_pointer(find_unsolved_matrix_row_index(layer,x_itor,y_itor+1),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno_P1, set_pointer);
	
	
	// Tvil,m,o
	if (datalibrary_->all_layers_info_[layer]->type_==UCOOL_LAYER || 
		datalibrary_->all_layers_info_[layer_above]->type_==UCOOL_LAYER || 
		datalibrary_->all_layers_info_[layer_below]->type_==UCOOL_LAYER) {
		
		
		if( datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER){
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			
			store_pointer(find_unsolved_matrix_row_index(layer+1,x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo, set_pointer);
			
		}else if(datalibrary_->all_layers_info_[layer]->type_==UCOOL_LAYER){
			store_pointer(find_unsolved_matrix_row_index(layer,x_itor, y_itor),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo, set_pointer);		
		}else{	//this means that datalibrary_->all_layers_info_[layer-1]==UCOOL_LAYER
			int x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			int y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			
			store_pointer(find_unsolved_matrix_row_index(layer,x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo, set_pointer);
		}
	}
	
	// Now go through the model again and generate rows for each of the units
	// T_inf m,n,o (this is the last element)
	//	store_pointer(datalibrary_->unsolved_matrix_dyn_->ncols()-1,y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tINF_mno, set_pointer);
}


//This will initialize the unsolved matrix (allocate it)
//Further, this will set the t_mn_o-type coefficient pointers to point to the correct location in the unsolved matrix
void sesctherm3Dmodel::initialize_unsolved_matrix(){
	int y_itor_global=0;
	
	int global_size=0;
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		if(datalibrary_->all_layers_info_[i]->layer_used_){
			global_size+=(datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->ncols()*datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->nrows()) - 
			datalibrary_->all_layers_info_[i]->unused_dyn_count_;
		}
	}
	
	datalibrary_->num_locktemp_rows_=0;
	//increase the global size by the number of nodes with temp locking enabled 
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_ && datalibrary_->all_layers_info_[layer]->temp_locking_enabled_){
			datalibrary_->num_locktemp_rows_+=(datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->ncols()*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_->nrows()) - datalibrary_->all_layers_info_[layer]->unused_dyn_count_;
		}
			
			
			
			//resize the unsolved_model_dyn_ to have the correct number of rows and columns
			//delete datalibrary_->unsolved_matrix_dyn_->clear(); //clear out of the model
			datalibrary_->unsolved_matrix_dyn_ = new Matrix(global_size, global_size);
	int size=datalibrary_->unsolved_matrix_dyn_->ncols();	
	
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					
					initialize_unsolved_matrix_row(y_itor_global, layer, x_itor, y_itor, false);   //allocate space within the unsolved matrix
					y_itor_global++;
					
				}
		}	
			
			
			y_itor_global=0;
			for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
				if(datalibrary_->all_layers_info_[layer]->layer_used_){
					for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
						for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
							if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
								continue;   //skip if unit is not defined
							initialize_unsolved_matrix_row(y_itor_global, layer, x_itor, y_itor, true);   //set the model pointers to the various elements
							y_itor_global++;
						}
				}	
					
					
}


void sesctherm3Dmodel::compute_rc_start(){
	datalibrary_->accumulate_rc_=true;		//accumulate datapoints
	
	//clear the datapoints arrays in each model unit
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					if(get_dyn_array(layer)[x_itor][y_itor].datapoints_x_.size()!=0){
						get_dyn_array(layer)[x_itor][y_itor].datapoints_x_.clear();
						get_dyn_array(layer)[x_itor][y_itor].datapoints_y_.clear();
					}
				}
		}
			
}

int sesctherm3Dmodel::get_modelunit_count(int layer){
	I(layer>=0 && (uint32_t)layer<datalibrary_->all_layers_info_.size());
	return (get_dyn_array(layer).nrows()*get_dyn_array(layer).ncols());
}
int sesctherm3Dmodel::get_modelunit_rows(int layer){
	I(layer>=0 && (uint32_t)layer<datalibrary_->all_layers_info_.size());
	return (get_dyn_array(layer).nrows());
}
int sesctherm3Dmodel::get_modelunit_cols(int layer){
	I(layer>=0 && (uint32_t)layer<datalibrary_->all_layers_info_.size());
	return (get_dyn_array(layer).ncols());
}

void sesctherm3Dmodel::set_time(double timestep){
	datalibrary_->timestep_=timestep;
}
void sesctherm3Dmodel::compute_rc_stop(){
	datalibrary_->accumulate_rc_=false;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					get_dyn_array(layer)[x_itor][y_itor].compute_rc();
					
					
				}
		}
}

void sesctherm3Dmodel::fast_forward(double timestep){
	datalibrary_->use_rc_=true;
	compute(timestep);
	datalibrary_->use_rc_=false;
}

void sesctherm3Dmodel::print_unsolved_model(){
	
				cerr << "******** Printing unsolved_matrix_dyn_: ********" << endl;
	cerr << "unsolved_matrix_dyn_.ncols()=" << datalibrary_->unsolved_matrix_dyn_->ncols() << endl;
	cerr << "unsolved_matrix_dyn_.nrows()=" << datalibrary_->unsolved_matrix_dyn_->nrows() << endl << endl;
	
	//Now we create the labels for all of the various units
	vector<string> tempVector;
	tempVector.push_back("0,");
	
	
	
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_)
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					tempVector.push_back("L" + sesctherm_utilities::stringify(layer) + "[" + sesctherm_utilities::stringify(x_itor) + "][" + sesctherm_utilities::stringify(y_itor) + "],");
				}
					//for(uint32_t itor=0;itor<tempVector.size();itor++)
					//	cerr << tempVector[itor];
					cerr << endl;
	
	Matrix_itor i,j;
	uint32_t itor=0;
	uint32_t itor2=1;
	
	for (j=0; j < (*datalibrary_->unsolved_matrix_dyn_).nrows(); ++j) {
		cerr  << tempVector[itor+1] << " ";
		itor2=1;
		for (i=0; i < (*datalibrary_->unsolved_matrix_dyn_).ncols(); ++i) {
			//if(!EQ((*datalibrary_->unsolved_matrix_dyn_)(j,i),0.0)){
			//	cerr << tempVector[itor2] << " ";
			//	cerr << setw(4) << (*datalibrary_->unsolved_matrix_dyn_)(j,i) << "[" << j << "," << i << "]" << "\t";
			cerr <<	(*datalibrary_->unsolved_matrix_dyn_)(j,i) << "\t";
			//}
			itor2++;
		}
		cerr << "\t [rhs: " << datalibrary_->temperature_matrix_[itor] << "]" << endl;
		itor++;
	}
}

//run through all of the floorplan dynamic arrays and calculate the terms for each one
void sesctherm3Dmodel::compute_model_units(bool calc_solution_only, double time_increment, bool accumulate_rc, bool use_rc) {
	
	
	
	
	//swap current temperature and previous temperature pointers
	//1) Previous Temperatures take on the value of the temperature array
	//	and Temperature array takes on previous temperatues
	//2) We then overwrite the temperatures array with the new temperatures
	
	SUElement_t* temp=datalibrary_->previous_temperature_matrix_;
	datalibrary_->previous_temperature_matrix_=datalibrary_->temperature_matrix_;
	datalibrary_->temperature_matrix_=temp;
	
	
	if(!calc_solution_only){
		recompute_material_properties();
		
		
	}
	
	/*
	//handle the locked temperatures
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_ && datalibrary_->all_layers_info_[layer]->temp_locking_enabled_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++){
					if(!get_dyn_array(layer)[x_itor][y_itor].defined_){
						continue;
					}
					//cerr << "temp locking enabled on layer: " << layer << " value is:" << datalibrary_->all_layers_info_[layer]->lock_temp_ << endl;
					*(get_dyn_array(layer)[x_itor][y_itor].get_previous_temperature())=datalibrary_->all_layers_info_[layer]->lock_temp_;
					//else{
					//	get_dyn_array(layer)[x_itor][y_itor].get_previous_temperature()=get_dyn_array(layer)[x_itor][y_itor].get_temperature();
					//}
				}
		}


*/

int y_itor_global=0;
for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
if(datalibrary_->all_layers_info_[layer]->layer_used_){
	for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
		for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
			if(!get_dyn_array(layer)[x_itor][y_itor].defined_)
				continue;
			
			get_dyn_array(layer)[x_itor][y_itor].calculate_terms(time_increment, calc_solution_only, use_rc);
			if(accumulate_rc)
				get_dyn_array(layer)[x_itor][y_itor].accumulate_rc();				
			y_itor_global++;
		}
}


//t_INF is always equal to ambient! (infinite reservoir of heat)
//datalibrary_->temperature_matrix_[datalibrary_->unsolved_matrix_dyn_->nrows()-1]=datalibrary_->config_data_->ambient_temp_;



if(!calc_solution_only) {
    //store the unsolved_matrix_dyn_ to the superLU matrix format (SuperMatrix at datalibrary_->A)
    createSuperLUMatrix();
}

}

void sesctherm3Dmodel::print_unsolved_model_row(int row){
	//Now we create the labels for all of the various units
	vector<string> tempVector;
	tempVector.push_back("0,");
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_)
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					tempVector.push_back("L" + sesctherm_utilities::stringify(layer) + "[" + sesctherm_utilities::stringify(y_itor) + "][" + sesctherm_utilities::stringify(x_itor) + "]");
				}
					//for(uint32_t itor=0;itor<tempVector.size();itor++)
					//	cerr << tempVector[itor];
					cerr << endl;
	
	Matrix_itor i,j;
	uint32_t itor=1;
	uint32_t itor2=1;
	cerr  << tempVector[row+1] << ":     ";
	for (i=0; i < (*datalibrary_->unsolved_matrix_dyn_).ncols()-1; ++i) {
		if(!EQ((*datalibrary_->unsolved_matrix_dyn_)(row,i),0.0)){
			cerr << tempVector[itor2] << ":";
			cerr << setw(4) << (*datalibrary_->unsolved_matrix_dyn_)(row,i) << "[" << row << "," << i << "]" << "     ";
		}
		itor2++;
	}
	cerr << endl;
}

void sesctherm3Dmodel::partition_floorplans() {	
	double bottomy_datapoint=0;
	double leftx_datapoint=0;	
	double rightx_datapoint=0;
    double topy_datapoint=0;
	double center_point =0;
    double width;
    double height;
    uint32_t y_itor=0;
    uint32_t x_itor=0;
    uint32_t x_itor_dyn_index=0;
    uint32_t y_itor_dyn_index=0;
    bool got_x_offset=false;
    bool got_y_offset=false;
    int32_t x_offset=0;
    int32_t y_offset=0;
	
	//clear the leftx datapoints
	leftx_.clear();
	
	//Step1: accumulate all of the datapoints to leftx and bottomy
	//run through all the layers 
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		if(datalibrary_->all_layers_info_[i]->layer_used_){
			if(datalibrary_->all_layers_info_[i]->type_==UCOOL_LAYER){
				for (uint32_t j=0;j<datalibrary_->all_layers_info_[i]->ucool_floorplan_->leftx_coords_.size();j++) {
					leftx_.push_back(datalibrary_->all_layers_info_[i]->ucool_floorplan_->leftx_coords_[j]);
				}
				for (uint32_t j=0;j<datalibrary_->all_layers_info_[i]->ucool_floorplan_->bottomy_coords_.size();j++) {		
					bottomy_.push_back(datalibrary_->all_layers_info_[i]->ucool_floorplan_->bottomy_coords_[j]);
				}
			}
			else if(datalibrary_->all_layers_info_[i]->type_==HEAT_SINK_FINS_LAYER){
				//store all the leftx datapoints from the heat sink fins
				//Add the rightx datapoints that correspond to the heatsink fins (we don't need datapoint 0)
				double leftx_datapoint=datalibrary_->all_layers_info_[i]->leftx_;
				
				for (int j=0;i<datalibrary_->config_data_->heat_sink_fin_number_;j++) {
					leftx_.push_back(leftx_datapoint);
					leftx_datapoint += datalibrary_->config_data_->heat_sink_width_ / datalibrary_->config_data_->heat_sink_fin_number_;
				}
				
				//store bottomy and topy datapoints
				bottomy_.push_back(datalibrary_->all_layers_info_[i]->bottomy_);
				bottomy_.push_back(datalibrary_->all_layers_info_[i]->bottomy_+datalibrary_->all_layers_info_[i]->height_);
				
			}	
			//if the layer has an associated floorplan and is NOT the UCOOL LAYER
			else if(datalibrary_->all_layers_info_[i]->type_!=UCOOL_LAYER && datalibrary_->all_layers_info_[i]->chip_floorplan_!=NULL){
				//store all the leftx datapoints from the chip floorplan
				for (uint32_t j=0;j<datalibrary_->all_layers_info_[i]->chip_floorplan_->leftx_coords_.size();j++) {
					leftx_.push_back(datalibrary_->all_layers_info_[i]->chip_floorplan_->leftx_coords_[j]);
				}
				
				//store all the bottomy datapoints from the chip floorplan
				for (uint32_t j=0;j<datalibrary_->all_layers_info_[i]->chip_floorplan_->bottomy_coords_.size();j++) {
					bottomy_.push_back(datalibrary_->all_layers_info_[i]->chip_floorplan_->bottomy_coords_[j]);
				}
				
			}			
			else{	
				//DIE_TRANSISTOR_LAYER must have an associated floorplan 
				if(datalibrary_->all_layers_info_[i]->type_==DIE_TRANSISTOR_LAYER){
					cerr << "FATAL: sesctherm3Dmodel::store_model_unit => DIE_TRANSISTOR_LAYER must have an associated floorplan defined (currently set to -1)" << endl;
					exit(1);
				}
				//store leftx and rightx datapoints
				leftx_.push_back(datalibrary_->all_layers_info_[i]->leftx_);
				leftx_.push_back(datalibrary_->all_layers_info_[i]->rightx_);
				//store bottomy and topy datapoints
				bottomy_.push_back(datalibrary_->all_layers_info_[i]->bottomy_);
				bottomy_.push_back(datalibrary_->all_layers_info_[i]->topy_);
			}
		}
	}
	
	//sort the vectors, remove duplicate entries
    std::sort(leftx_.begin(),leftx_.end());
    std::sort(bottomy_.begin(),bottomy_.end());
	
    vector<double>::iterator new_end = std::unique(leftx_.begin(),leftx_.end(),sesctherm_utilities::myUnique());
    // delete all elements past new_end
    leftx_.erase(new_end, leftx_.end());
	
    new_end= std::unique(bottomy_.begin(),bottomy_.end(),sesctherm_utilities::myUnique());
    // delete all elements past new_end
    bottomy_.erase(new_end, bottomy_.end());
	
	
	//Step2: Create the 2-d cross-sectional regions for each layer
	//run through all the layers 
	cerr << endl;
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		if(datalibrary_->all_layers_info_[i]->layer_used_){
#ifdef _SESCTHERM_DEBUG
			cerr << "Partitioning Layer: " << i << endl;
#endif
			rightx_datapoint=datalibrary_->all_layers_info_[i]->leftx_;
			topy_datapoint=datalibrary_->all_layers_info_[i]->bottomy_;
			x_offset=0;
			y_offset=0;
			y_itor=0;
			x_itor=0;
			y_itor_dyn_index=0;
			x_itor_dyn_index=0;
			got_y_offset=false;
			got_x_offset=false;
			
			//find the x-offset. This will be the index of leftx_[] that is just beyond the first leftx_ coordinate of the particular layer
			while (LE(leftx_[x_offset],datalibrary_->all_layers_info_[i]->leftx_)){
				x_offset++;
				if(x_offset==(int)leftx_.size() || GT(leftx_[x_offset], datalibrary_->all_layers_info_[i]->rightx_)){
					cerr << "Sesctherm3DModel::partitionfloorplans -> Fatal: Could not find datapoint just beyond leftx_ for floorplan [ " << i << " ] " << endl;
					exit(1);
				}
			}
			
			//find the y-offset. This will be the index of bottomy_[] that is just beyond the first bottomy_ coordinate of the particular layer
			while (LE(bottomy_[y_offset],datalibrary_->all_layers_info_[i]->bottomy_)){
				y_offset++;
				if(y_offset==(int)bottomy_.size() || GT(bottomy_[x_offset], datalibrary_->all_layers_info_[i]->topy_) ){
					cerr << "Sesctherm3DModel::partitionfloorplans -> Fatal: Could not find datapoint just beyond bottomy_ for floorplan [ " << i << " ] " << endl;
					exit(1);
				}
			}
			
			
			
			//iterate from bottomy to topy
			while (LT(topy_datapoint,datalibrary_->all_layers_info_[i]->topy_)) {
				//store the bottomy datapoint of the cross-sectional region
				bottomy_datapoint=topy_datapoint;
				
				//if the bottom of the cross-sectional region occurs in an overlapping region with another layer with higher resolution, select the layer with highest resolution
				//find the finest granularity for the particular cross-sectional region
				double min_offset=0;
				for (int j=0; j < (int)datalibrary_->all_layers_info_.size(); j++) {
					if (GE(bottomy_datapoint,datalibrary_->all_layers_info_[j]->bottomy_) && 
						LE(bottomy_datapoint,datalibrary_->all_layers_info_[j]->topy_)){
						//if min_offset not yet assigned, just assign the first encountered resolution (not necessary the highest resolution)
						if(min_offset==0)
							min_offset=datalibrary_->all_layers_info_[j]->granularity_;
						else
							min_offset=MIN(min_offset,datalibrary_->all_layers_info_[j]->granularity_);
					}
				}
				//if min offset equals zero, then no offset was assigned. This means that an error occurred as every region's topy should occur within SOME layer
				if(min_offset==0){
					cerr << "Sesctherm3DModel::partitionfloorplans -> Fatal: Could not determine highest granularity for the particular cross-sectional region in layer [" << i << "]" << endl;
					exit(1);
				}
				
				//offset the topy_datapoint
				topy_datapoint+=min_offset;
				
				//find the topy_datapoint of the cross-sectional region
				//if there is very fine granularity, this may be a lower value than the next flp unit's topy coordinate
				topy_datapoint=MIN(bottomy_[y_itor+y_offset],topy_datapoint);
				//topy_datapoint - bottomy_datapoint
				height=topy_datapoint-bottomy_datapoint; //compute the height of the cross-sectional region
				x_itor=0;
				x_itor_dyn_index=0;
				
				rightx_datapoint=datalibrary_->all_layers_info_[i]->leftx_;	//the rightx_datapoint is initially the left point of the particular layer
				
				
				//iterate from leftx to rightx
				while (LT(rightx_datapoint,datalibrary_->all_layers_info_[i]->rightx_)) {
					//store the leftx datapoint of the cross-sectional region
					leftx_datapoint=rightx_datapoint;
					//if the left of the cross-sectional region occurs in an overlapping region with another layer with higher resolution, select the layer with highest resolution
					double min_offset=0;
					for (int j=0; j < (int)datalibrary_->all_layers_info_.size(); j++) {
						if (GE(rightx_datapoint,datalibrary_->all_layers_info_[j]->leftx_) && LE(rightx_datapoint,datalibrary_->all_layers_info_[j]->rightx_)){
							//if min_offset not yet assigned, just assign the first encountered resolution (not necessary the highest resolution)
							if(min_offset==0)
								min_offset=datalibrary_->all_layers_info_[j]->granularity_;
							else
								min_offset=MIN(min_offset,datalibrary_->all_layers_info_[j]->granularity_);
						}
					}
					//if min offset equals zero, then no offset was assigned. This means that an error occurred as every region's topy should occur within SOME layer
					if(min_offset==0){
						cerr << "Sesctherm3DModel::partitionfloorplans -> Fatal: Could not determine highest granularity for the particular cross-sectional region in layer [" << i <<"]" << endl;
						exit(1);
					}
					
					
					//offset the rightx_datapoint
					rightx_datapoint+=min_offset;	
					//find the rightx_datapoint of the cross-sectional region
					//if there is very fine granularity, this may be a lower value than the next flp unit's leftx coordinate			
					rightx_datapoint=MIN(leftx_[x_itor+x_offset],rightx_datapoint);
					width=rightx_datapoint-leftx_datapoint; //compute the width of the cross-sectional region
					
					store_model_unit(rightx_datapoint, topy_datapoint, width, height, x_itor_dyn_index, y_itor_dyn_index, i);
					
					if (GE(rightx_datapoint,leftx_[x_itor+x_offset])) //if we haven't reached the data value leftx_[x_itor], don't increment itor
						x_itor++;
					x_itor_dyn_index++;
				}
				if (GE(topy_datapoint,bottomy_[y_itor+y_offset])) //if we haven't reached the data value bottomy_[y_itor], don't increment itor
					y_itor++;
				y_itor_dyn_index++;
			}
		}
	}	
	
	
    //Step 3: Run through all of the layers within the dynamic arrays, fill in all additional information
	//FIXME: HEAT_SPREADER_LAYER is not HANDLED right now (disabled regions between the fins are NOT properly handled)
	
	//NOTE: no need at deal with conduct_center_(left,right,etc), specific_heat/row/alpha or x2 (handled in store_model_unit)
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		if(datalibrary_->all_layers_info_[i]->layer_used_){
			sesctherm3Dlayerinfo& layer_above = *datalibrary_->all_layers_info_[MIN(i+1,(int)datalibrary_->all_layers_info_.size()-1)];
			sesctherm3Dlayerinfo& layer_below = *datalibrary_->all_layers_info_[MAX(i-1,0)];
			for (y_itor=0;y_itor<get_dyn_array(i).nrows();y_itor++) {
				for (x_itor=0;x_itor<get_dyn_array(i).ncols();x_itor++) {
					
					model_unit& cur_model_unit=get_dyn_array(i)[x_itor][y_itor];
					double center_point_x=(cur_model_unit.leftx_+cur_model_unit.width_)/2;
					double center_point_y=(cur_model_unit.leftx_+cur_model_unit.height_)/2;
					
					//FIXME: add ability for detailed interface resistances between thermal units
					cur_model_unit.resist_left_=0;
					cur_model_unit.resist_right_=0;
					cur_model_unit.resist_bottom_=0;
					cur_model_unit.resist_top_=0;
					cur_model_unit.resist_down_=0;
					cur_model_unit.resist_up_=0;
					
					if(datalibrary_->all_layers_info_[i]->type_==UCOOL_LAYER){
						cur_model_unit.conduct_top_=0;
						cur_model_unit.conduct_bottom_=0;
						cur_model_unit.conduct_left_=0;
						cur_model_unit.conduct_right_=0;
						cur_model_unit.conduct_down_=0;
						cur_model_unit.conduct_up_=0;
						cur_model_unit.x1_=0;
						cur_model_unit.x3_=0;
						cur_model_unit.y1_=0;
						cur_model_unit.y3_=0;
						cur_model_unit.z1_=0;
						cur_model_unit.z3_=0;
					}
					
					double leftx=cur_model_unit.leftx_;
					double bottomy=cur_model_unit.bottomy_;
					
					//DETERMINE VERTICAL PARAMETERS (THICKNESSES and CONDUCTIVITIES)		
					if(i-1 >= 0){	//if there is a layer below, and a unit below
						
						model_unit* found_unit= model_unit::find_model_unit(cur_model_unit.leftx_,cur_model_unit.bottomy_,i-1,datalibrary_);
						if(found_unit == NULL){ //there is no layer unit below this one, treat as insulating
							cur_model_unit.y1_=0;
							cur_model_unit.conduct_bottom_=0; 
							cur_model_unit.heat_transfer_methods_bottom_=0;
							cur_model_unit.model_bottom_=0;
						}
						else{
							//if the layer is the heat_sink and the layer below is a heat_spreader_layer, then add the interface resistance
							if(datalibrary_->all_layers_info_[i]->type_==HEAT_SINK_LAYER && layer_below.type_==HEAT_SPREADER_LAYER){
								cur_model_unit.resist_bottom_=datalibrary_->config_data_->heat_sink_resistance_;
							}
							cur_model_unit.y1_=found_unit->y2_;
							cur_model_unit.conduct_bottom_=found_unit->conduct_center_top_; 
							cur_model_unit.heat_transfer_methods_bottom_=determine_heat_transfer_methods(cur_model_unit,*found_unit);
							cur_model_unit.model_bottom_=found_unit;							
						}
						
					} //this is the bottom layer, treat bottom as insulating
					else {
						cur_model_unit.conduct_bottom_=0; //bottom of the bottom layer  is considered insulated
						cur_model_unit.y1_=0;			//y1 is 0 (insulating, not convection cooling)
						cur_model_unit.heat_transfer_methods_bottom_=0;
						cur_model_unit.model_bottom_=0;
					}
					
					//if there is a layer above, and a unit above
					if(i+1 < (int)datalibrary_->all_layers_info_.size()){
						
						model_unit* found_unit= model_unit::find_model_unit(cur_model_unit.leftx_,cur_model_unit.bottomy_,i+1,datalibrary_);
						if(found_unit == NULL){ //there is no layer unit above this one, treat as insulating
							cur_model_unit.y3_=0;
							cur_model_unit.conduct_top_=0; 
							cur_model_unit.heat_transfer_methods_top_=0;
							cur_model_unit.model_top_=0;
						}
						else{
							//if the layer is the heat_spreader and the layer above is a heat_sink_layer, then add the interface resistance
							if(datalibrary_->all_layers_info_[i]->type_==HEAT_SPREADER_LAYER && layer_above.type_==HEAT_SINK_LAYER){
								cur_model_unit.resist_top_=datalibrary_->config_data_->heat_sink_resistance_;
							}
							cur_model_unit.y3_=found_unit->y2_;
							cur_model_unit.conduct_top_=found_unit->conduct_center_bottom_; 
							cur_model_unit.heat_transfer_methods_top_=determine_heat_transfer_methods(cur_model_unit,*found_unit);			
							cur_model_unit.model_top_=found_unit;
						}
						
					} //this is the top layer, treat top as insulating
					else{
						cur_model_unit.conduct_top_=0; //top of the top layer is considered insulated 
						cur_model_unit.y3_=0;			
						cur_model_unit.heat_transfer_methods_top_=0;
						cur_model_unit.model_top_=0;
					}
					
					
					//DETERMINE LATERAL PARAMETERS (WIDTH/HEIGHT and CONDUCTIVITIES)
					
					//if there is a unit downwards
					if ((int)(y_itor-1) >= 0) {
						get_dyn_array(i)[x_itor][y_itor].conduct_down_=get_dyn_array(i)[x_itor][y_itor-1].conduct_center_up_;				
						get_dyn_array(i)[x_itor][y_itor].z1_=get_dyn_array(i)[x_itor][y_itor-1].z2_;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_down_=determine_heat_transfer_methods(get_dyn_array(i)[x_itor][y_itor],get_dyn_array(i)[x_itor][y_itor-1]);			
						get_dyn_array(i)[x_itor][y_itor].model_down_=&get_dyn_array(i)[x_itor][y_itor-1];
					}
					else{
						get_dyn_array(i)[x_itor][y_itor].conduct_down_=0;
						get_dyn_array(i)[x_itor][y_itor].z1_=0;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_down_=0;
						get_dyn_array(i)[x_itor][y_itor].model_down_=0;
					}
					
					//if there is a unit upwards
					if (y_itor+1 < get_dyn_array(i).nrows()) {
						get_dyn_array(i)[x_itor][y_itor].conduct_up_=get_dyn_array(i)[x_itor][y_itor+1].conduct_center_down_;				
						get_dyn_array(i)[x_itor][y_itor].z3_=get_dyn_array(i)[x_itor][y_itor+1].z2_;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_up_=determine_heat_transfer_methods(get_dyn_array(i)[x_itor][y_itor],get_dyn_array(i)[x_itor][y_itor+1]);			
						get_dyn_array(i)[x_itor][y_itor].model_up_=&get_dyn_array(i)[x_itor][y_itor+1];
					}
					else{
						get_dyn_array(i)[x_itor][y_itor].conduct_up_=0;
						get_dyn_array(i)[x_itor][y_itor].z3_=0;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_up_=0;
						get_dyn_array(i)[x_itor][y_itor].model_up_=0;
					}
					
					//if there is a unit to the left
					if ((int)(x_itor-1) >= 0) {
						get_dyn_array(i)[x_itor][y_itor].conduct_left_=get_dyn_array(i)[x_itor-1][y_itor].conduct_center_right_;				
						get_dyn_array(i)[x_itor][y_itor].x1_=get_dyn_array(i)[x_itor-1][y_itor].x2_;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_left_=determine_heat_transfer_methods(get_dyn_array(i)[x_itor][y_itor],get_dyn_array(i)[x_itor-1][y_itor]);									
						get_dyn_array(i)[x_itor][y_itor].model_left_=&get_dyn_array(i)[x_itor-1][y_itor];
					}
					else{
						get_dyn_array(i)[x_itor][y_itor].conduct_left_=0;
						get_dyn_array(i)[x_itor][y_itor].x1_=0;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_left_=0;
						get_dyn_array(i)[x_itor][y_itor].model_left_=0;
					}
					
					
					//if there is a unit to the right
					if (x_itor+1 <  get_dyn_array(i).ncols()) {
						get_dyn_array(i)[x_itor][y_itor].conduct_right_=get_dyn_array(i)[x_itor+1][y_itor].conduct_center_left_;				
						get_dyn_array(i)[x_itor][y_itor].x3_=get_dyn_array(i)[x_itor+1][y_itor].x2_;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_right_=determine_heat_transfer_methods(get_dyn_array(i)[x_itor][y_itor],get_dyn_array(i)[x_itor+1][y_itor]);			
						get_dyn_array(i)[x_itor][y_itor].model_right_=&get_dyn_array(i)[x_itor+1][y_itor];
					}
					else{
						get_dyn_array(i)[x_itor][y_itor].conduct_right_=0;
						get_dyn_array(i)[x_itor][y_itor].x3_=0;
						get_dyn_array(i)[x_itor][y_itor].heat_transfer_methods_right_=0;
						get_dyn_array(i)[x_itor][y_itor].model_right_=0;
					}									
				}
			}
		}
    }
	
	//here we check to see if any of the conduct_ center/left/top/bottom parameters are zero
	//if one of these parameters is zero, this means that the conduct_left/right/top/bottom parameter in any unit that will 
	//come in contact with that unit is also zero. Hence, it doesn't matter what the conduct-center value is in this case
	//However, we need for ALL the conduct_center-type parameters to be nonzero or we will get infinite values in the equations!
	//hence we set all the parameters to -1
	/*	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++)
		if(datalibrary_->all_layers_info_[i]->layer_used_)
		for (y_itor=0;y_itor<get_dyn_array(i).nrows();y_itor++) 
		for (x_itor=0;x_itor<get_dyn_array(i).ncols();x_itor++) {
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_left_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_left_=-1;
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_right_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_right_=-1;
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_top_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_top_=-1;
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_bottom_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_bottom_=-1;
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_up_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_up_=-1;
			if(get_dyn_array(i)[x_itor][y_itor].conduct_center_down_==0)
				get_dyn_array(i)[x_itor][y_itor].conduct_center_down_=-1;
			
		}
	*/
}

uint32_t sesctherm3Dmodel::determine_heat_transfer_methods(model_unit& model_unit_source,model_unit& model_unit_dest){
	//make sure to set the convection coefficient properly
	if(model_unit_dest.heat_transfer_methods_center_&(1<<HEAT_CONVECTION_TRANSFER)){
		model_unit_source.convection_coefficient_=model_unit_dest.convection_coefficient_;
	}
	//if the source model unit is a fluid/air that uses convective heat transfer, it CANNOT use conduction regardless of what the adjavent unit is
	if(model_unit_source.heat_transfer_methods_center_&(1<<HEAT_CONVECTION_TRANSFER)){
		return(1<<HEAT_CONVECTION_TRANSFER);
	}
	else{
		return(model_unit_dest.heat_transfer_methods_center_);
	}
}
//(x,y) is the datapoint at the very center of the model_unit
double sesctherm3Dmodel::get_governing_equation(double rightx, double topy, double width, double height, int32_t layer) {
	double leftx=rightx-width;
	double bottomy=topy-height;
	//check to see if the layer below is a ucool layer
	//set the correct equation accordingly
	//we assume that the ucooler is positioned such that heat is dissipated toward the heatsink (upwards)
	//hence if the ucool layer is below the given layer then the equation is a HOT equation
	//FIXME: this doesn't work if there are two ucool layers back-to-back
	if (layer >= 1 &&
		datalibrary_->all_layers_info_[layer-1]->type_==UCOOL_LAYER &&
		datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_ != NULL &&
		datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2))))
		return(UCOOL_HOT_EQUATION);
	
	//check to see if the layer above is a ucool layer
	//set the correct equation accordingly
	if (layer < (int)datalibrary_->all_layers_info_.size()-1 &&
		datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER &&
		datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_ != NULL &&
		datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2))))
		return(UCOOL_COLD_EQUATION);
	
	
    double itor=0;
    switch (datalibrary_->all_layers_info_[layer]->type_) {
		case MAINBOARD_LAYER:
		case PINS_LAYER:
		case PWB_LAYER:
		case FCPBGA_LAYER:
		case C4_UNDERFILL_LAYER:
		case INTERCONNECT_LAYER:
		case DIE_TRANSISTOR_LAYER:
		case BULK_SI_LAYER:
		case OIL_LAYER:
		case AIR_LAYER:
		case HEAT_SPREADER_LAYER:
		case HEAT_SINK_LAYER:
			return(BASIC_EQUATION);
			
		case UCOOL_LAYER:
			return(UCOOL_INNER_LAYER_EQUATION); //ucooler inner node
			break;
			
		case HEAT_SINK_FINS_LAYER:
			bottomy=topy-height;
			leftx=rightx-width;
			itor=0;
			while (LT(itor,(leftx+(width/2)))) {
				itor+=(double)datalibrary_->config_data_->heat_sink_width_/datalibrary_->config_data_->heat_sink_fin_number_;
			}
				itor-=(double)datalibrary_->config_data_->heat_sink_width_/datalibrary_->config_data_->heat_sink_fin_number_;
			itor*=(double)datalibrary_->config_data_->heat_sink_fin_number_;
			itor/=(double)datalibrary_->config_data_->heat_sink_width_;
			
			if (EVEN((int)itor)) 
				return(BASIC_EQUATION);
			else 
				return(-1.00);  //we are in between the heatsink fins (return -1.00)
		default:
			cerr << "sesctherm3Dmodel::get_governing_equation => unknown layer type [" << datalibrary_->all_layers_info_[layer]->type_ << "]" << endl;
			exit(1);
    }
}

void sesctherm3Dmodel::store_model_unit(double rightx, double topy, double width, double height, int32_t x_itor, int32_t y_itor, int32_t layer) {
	model_unit newunit(datalibrary_);
	double leftx=rightx-width;
	double bottomy=topy-height;
	
	//store a model_unit to the heat_sink layer dynamic_array ( no need to lookup anything for chip floorplan):
	   newunit.defined_=true;
	   newunit.x_coord_=x_itor;
	   newunit.y_coord_=y_itor;
	   newunit.leftx_=rightx-width;
	   newunit.bottomy_=topy-height;
	   newunit.topy_=topy;
	   newunit.rightx_=rightx;
	   newunit.width_=width;
	   newunit.height_=height;        
	   newunit.defined_=true;
	   /* if(datalibrary_->all_layers_info_[layer]->temp_locking_enabled_){
		   newunit.get_previous_temperature()=datalibrary_->all_layers_info_[layer]->lock_temp_;
	   newunit.get_temperature()=datalibrary_->all_layers_info_[layer]->lock_temp_;
	   newunit.get_temperature()=datalibrary_->all_layers_info_[layer]->lock_temp_;
	   }
else{
		   newunit.get_previous_temperature()=datalibrary_->config_data_->init_temp_;
		   newunit.get_temperature()=datalibrary_->config_data_->init_temp_;
		   newunit.get_temperature()=datalibrary_->config_data_->init_temp_;	
}
*/
	   newunit.flp_percentage_=1; //100%
	   newunit.ucool_percentage_=0; //0%
	   
	   newunit.x2_=width;
	   newunit.y2_=datalibrary_->all_layers_info_[layer]->thickness_;
	   newunit.z2_=height;
	   
	   // newunit.name_=datalibrary_->all_layers_info_[layer]->name_;
	   newunit.governing_equation_=get_governing_equation(rightx,topy, width, height, layer);
	   
	   newunit.specific_heat_=datalibrary_->all_layers_info_[layer]->spec_heat_;
	   newunit.row_=datalibrary_->all_layers_info_[layer]->density_;
	   newunit.conduct_center_left_=datalibrary_->all_layers_info_[layer]->horizontal_conductivity_;
	   newunit.conduct_center_right_=datalibrary_->all_layers_info_[layer]->horizontal_conductivity_;
	   newunit.conduct_center_down_=datalibrary_->all_layers_info_[layer]->horizontal_conductivity_;
	   newunit.conduct_center_up_=datalibrary_->all_layers_info_[layer]->horizontal_conductivity_;
	   newunit.conduct_center_top_=datalibrary_->all_layers_info_[layer]->vertical_conductivity_up_;
	   newunit.conduct_center_bottom_=datalibrary_->all_layers_info_[layer]->vertical_conductivity_down_;		
	   
	   newunit.emissivity_=datalibrary_->all_layers_info_[layer]->emissivity_;
	   newunit.convection_coefficient_=datalibrary_->all_layers_info_[layer]->convection_coefficient_;
	   newunit.heat_transfer_methods_center_=datalibrary_->all_layers_info_[layer]->heat_transfer_methods_;
	   newunit.temp_locking_enabled_=datalibrary_->all_layers_info_[layer]->temp_locking_enabled_;
	   newunit.lock_temp_=datalibrary_->all_layers_info_[layer]->lock_temp_;
	   
	   //store a mapping of [leftx_,bottomy_] to [x_itor][y_itor]
	   datalibrary_->all_layers_info_[layer]->coord_to_index_x_[newunit.leftx_]=x_itor;
	   datalibrary_->all_layers_info_[layer]->coord_to_index_y_[newunit.bottomy_]=y_itor;
	   
	   
	   //If we are in the vacant region, ignore
	   if (get_governing_equation(rightx, topy, width, height, layer)==-1) {
		   newunit.governing_equation_=-1.0;
		   newunit.defined_=false;
		   newunit.flp_percentage_=0;
		   return;
	   }		
	   
	   
	   
	   switch(datalibrary_->all_layers_info_[layer]->type_){
		   case OIL_LAYER:
			   break;
		   case AIR_LAYER:
			   break;
		   case HEAT_SPREADER_LAYER:
			   break;
		   case HEAT_SINK_FINS_LAYER:
			   //this is the percentage of the particular heat sink fin
			   //area of cross-sectional region/size of fin
			   newunit.flp_percentage_=(width*height)/((datalibrary_->all_layers_info_[layer]->height_
														* datalibrary_->all_layers_info_[layer]->width_)/datalibrary_->config_data_->heat_sink_fin_number_);
			   break;
		   case MAINBOARD_LAYER:
		   case PINS_LAYER:
		   case PWB_LAYER:
		   case FCPBGA_LAYER:
		   case C4_UNDERFILL_LAYER:
		   case INTERCONNECT_LAYER:
		   case DIE_TRANSISTOR_LAYER:
		   case BULK_SI_LAYER:
			   //check if there is an associated floorplan defined
			   if(datalibrary_->all_layers_info_[layer]->chip_floorplan_!=NULL){
				   if (datalibrary_->all_layers_info_[layer]->chip_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2)))) {
					   newunit.source_chip_flp_=&datalibrary_->all_layers_info_[layer]->chip_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2)));
					   // newunit.name_=datalibrary_->all_layers_info_[layer]->chip_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).name_;
					   newunit.flp_percentage_=height*width/
						   (datalibrary_->all_layers_info_[layer]->chip_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).height_*
							datalibrary_->all_layers_info_[layer]->chip_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).width_);
				   } else {
					   cerr << "sesctherm3Dmodel::store_model_unit => Fatal:Chip region was found that does not correspond to a functional unit" << endl;
					   cerr << "sesctherm3Dmodel::store_model_unit => Could not find flp unit that corresponds to the datapoint [" << (leftx+(width/2)) << "][" << (bottomy+(height/2)) << "]" << endl;
					   exit(1);
				   }
			   }
			   break;
			   
		   case UCOOL_LAYER:
			   //virtual layer has no clearly defined material properties (assuming same as chip subtrate)
			   newunit.y2_=1; //virtual layer has thickness of 1(basically just ignore y2_ parameter in gamma then)
			   newunit.ucool_percentage_=0;
			   newunit.flp_percentage_=1;
			   if (datalibrary_->all_layers_info_[layer]->ucool_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2)))) {
				   newunit.source_ucool_flp_=&datalibrary_->all_layers_info_[layer]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2)));
				   // newunit.name_=datalibrary_->all_layers_info_[layer]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).name_;
				   newunit.ucool_percentage_=height*width/
					   (datalibrary_->all_layers_info_[layer]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).height_*
						datalibrary_->all_layers_info_[layer]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).width_);
				   newunit.source_ucool_flp_->model_unit_percentages_.push_back(newunit.ucool_percentage_); //accumulate all the various percentages in the source ucool flp
				   newunit.source_ucool_flp_->model_units_.push_back(&newunit);
			   } else {
				   newunit.defined_=false; //if the virtual layer model_unit does not correspond to any ucooler, set the model_unit as undefined
				   newunit.source_ucool_flp_=NULL;
				   // newunit.name_="";
				   newunit.ucool_percentage_=0;
			   }
				   newunit.conduct_center_left_=0;
			   newunit.conduct_center_right_=0;
			   newunit.conduct_center_down_=0;
			   newunit.conduct_center_up_=0;
			   newunit.conduct_center_top_=0;
			   newunit.conduct_center_bottom_=0;			
			   break;
			   
		   default:
			   cerr << "Fatal: store_model_unit: invalid layer number[" << layer << endl;
			   exit(1);
	   }
	   
	   
	   
	   //To handle the UCOOL_HOT and UCOOL_COLD units, we need to check the adjacent regions
	   //The reason why is because it might be possible for a UCOOL_HOT unit to appear in ANY other kind of layer
	   //To handle this, we need to be able to convert that unit into a UCOOL_HOT or cold unit
	   
	   //If the layer below is a UCOOL_LAYER, then this is a COLD region 
	   if( layer >= 1 &&  datalibrary_->all_layers_info_[layer-1]->type_ == UCOOL_LAYER){
		   if(datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_ == NULL){
			   cerr << "FATAL:sesctherm3Dmodel::store_model_unit => ucool layer [" << layer << "] has a null ucool_floorplan." << endl;
			   exit(1);
		   }
		   if (datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2)))) {
			   newunit.source_ucool_flp_=&datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2)));
			   // newunit.name_=datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).name_;
			   newunit.ucool_percentage_=height*width/
				   (datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).height_*
					datalibrary_->all_layers_info_[layer-1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).width_);
			   
		   }
	   }
	   
	   
	   if( layer < (int)(datalibrary_->all_layers_info_.size() - 1)  &&  datalibrary_->all_layers_info_[layer+1]->type_ == UCOOL_LAYER){
		   if(datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_ == NULL){
			   cerr << "FATAL:sesctherm3Dmodel::store_model_unit => ucool layer [" << layer << "] has a null ucool_floorplan." << endl;
			   exit(1);
		   }
		   if (datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->flp_unit_exists((leftx+(width)/2),(bottomy+(height/2)))) {
			   newunit.source_ucool_flp_=&datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2)));
			   //  newunit.name_=datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).name_;
			   newunit.ucool_percentage_=height*width/
				   (datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).height_*
					datalibrary_->all_layers_info_[layer+1]->ucool_floorplan_->find_flp_unit((leftx+(width)/2),(bottomy+(height/2))).width_);
		   }
	   }
	   
	   
	   get_dyn_array(layer)[x_itor][y_itor]=newunit;
	   //calculate the material properties for the particular unit
	   //depending upon the type, no computation may be required
	   sesctherm3Dmaterial::calc_material_properties_dyn_density(layer, x_itor, y_itor,datalibrary_);
}

//This will recompute the material conductivities for each 
//cross-sectional region based upon the material and temperature at that particular region
//at this point, only the silicon layer is temperature dependent
void sesctherm3Dmodel::recompute_material_properties(){
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
					sesctherm3Dmaterial::calc_material_properties_dyn_temp(layer, x_itor, y_itor, datalibrary_);				
				}
		}
}

void sesctherm3Dmodel::recompute_material_properties(int layer){
	if(layer<0 || layer>=(int)datalibrary_->all_layers_info_.size()){
		cerr << "FATAL: sesctherm3Dmodel::recompute_material_properties =>  layer[ " << layer << " ] does not exist" << endl;
		exit(1);
	}
		for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
            for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
                if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
                    continue;   //skip if unit is not defined
				sesctherm3Dmaterial::calc_material_properties_dyn_temp(layer, x_itor, y_itor, datalibrary_);				
            }
}
dynamic_array<model_unit>& sesctherm3Dmodel::get_dyn_array(int layer)
{
	if(layer<0 || layer>=(int)datalibrary_->all_layers_info_.size()){
		cerr << "FATAL: sesctherm3Dmodel::get_dyn_array =>  layer[ " << layer << " ] does not exist" << endl;
		exit(1);
	}
	return(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_);
}

void sesctherm3Dmodel::print_graphics(){
	
	datalibrary_->graphics_->print_graphics();
}

void sesctherm3Dmodel::accumulate_sample_data(){
	//accumulate data*timestep values in each of the layers
	for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++)
		datalibrary_->all_layers_info_[i]->accumulate_data_for_sample();
}

void sesctherm3Dmodel::compute_sample(){
	for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++)
		datalibrary_->all_layers_info_[i]->compute_sample();
	
}

void sesctherm3Dmodel::enable_temp_locking(int layer, double temperature){
	datalibrary_->all_layers_info_[layer]->temp_locking_enabled_=true;
	datalibrary_->all_layers_info_[layer]->lock_temp_=temperature;
}

void sesctherm3Dmodel::disable_temp_locking(int layer){
	datalibrary_->all_layers_info_[layer]->temp_locking_enabled_=false;
}

double sesctherm3Dmodel::get_max_timestep(){
	//stability criteria requires that finite-difference form of Fourier number be less than (1/6)
	//Fourier number is defined as alpha_*dt/(dx^2*dy^2*dz^2)
	//Therefore dt< ((1/6)*dx^2*dy^2*dz^2/alpha)
	double average_timestep=0;
	double num_elements=0;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
									//double volume=get_dyn_array(layer)[x_itor][y_itor].x2_*get_dyn_array(layer)[x_itor][y_itor].y2_*get_dyn_array(layer)[x_itor][y_itor].z2_;
					
					double ave_side_length=(get_dyn_array(layer)[x_itor][y_itor].x2_+get_dyn_array(layer)[x_itor][y_itor].y2_+get_dyn_array(layer)[x_itor][y_itor].z2_)/3;
					double volume=pow(ave_side_length,3.0);
					double surface_area=6*ave_side_length;
					//2*get_dyn_array(layer)[x_itor][y_itor].x2_*get_dyn_array(layer)[x_itor][y_itor].y2_+
					//2*get_dyn_array(layer)[x_itor][y_itor].z2_*get_dyn_array(layer)[x_itor][y_itor].y2_+
					//2*get_dyn_array(layer)[x_itor][y_itor].z2_*get_dyn_array(layer)[x_itor][y_itor].x2_;
					double char_length=volume/surface_area;
					average_timestep+=(1.0/6.0)*pow(char_length,2.0)/get_dyn_array(layer)[x_itor][y_itor].calc_alpha_max(false);
					num_elements++;
				}
		}
			return(average_timestep/num_elements);
}

double sesctherm3Dmodel::get_recommended_timestep(){
	//stability criteria requires that finite-difference form of Fourier number be less than (1/24)
	//Fourier number is defined as alpha_*dt/(dx^2*dy^2*dz^2)
	//Therefore dt< ((1/24)*dx^2*dy^2*dz^2/alpha)
	double average_timestep=0;
	double num_elements=0;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						continue;   //skip if unit is not defined
									//double volume=get_dyn_array(layer)[x_itor][y_itor].x2_*get_dyn_array(layer)[x_itor][y_itor].y2_*get_dyn_array(layer)[x_itor][y_itor].z2_;
					
					double ave_side_length=(get_dyn_array(layer)[x_itor][y_itor].x2_+get_dyn_array(layer)[x_itor][y_itor].y2_+get_dyn_array(layer)[x_itor][y_itor].z2_)/3;
					double volume=pow(ave_side_length,3.0);
					double surface_area=6*ave_side_length;
					//2*get_dyn_array(layer)[x_itor][y_itor].x2_*get_dyn_array(layer)[x_itor][y_itor].y2_+
					//2*get_dyn_array(layer)[x_itor][y_itor].z2_*get_dyn_array(layer)[x_itor][y_itor].y2_+
					//2*get_dyn_array(layer)[x_itor][y_itor].z2_*get_dyn_array(layer)[x_itor][y_itor].x2_;
					double char_length=volume/surface_area;
					average_timestep+=(1.0/24.0)*pow(char_length,2.0)/get_dyn_array(layer)[x_itor][y_itor].calc_alpha_max(false);
					num_elements++;
				}
		}	
			return(average_timestep/num_elements);
}



/*
 //here we take all the model units and generate an unsolved matrix
 void sesctherm3Dmodel::create_unsolved_matrix() {
	 int32_t y_itor_global=0;
	 
	 int32_t global_size=0;
	 
	 for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		 if(datalibrary_->all_layers_info_[i]->layer_used_){
			 global_size+=(datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->ncols()*datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_->nrows()) - 
			 datalibrary_->all_layers_info_[i]->unused_dyn_count_;
		 }
	 }
	 
	 
	 global_size++;  //this adds a node for the T_inf (the last node)
	 
	 //resize the unsolved_model_dyn_ to have the correct number of rows and columns
	 //delete datalibrary_->unsolved_matrix_dyn_->clear(); //clear out of the model
	 datalibrary_->unsolved_matrix_dyn_ = new Matrix(global_size, global_size);
	 // datalibrary_->unsolved_matrix_dyn_->increase_size(global_size,global_size); //resize to the proper size
	 //now set the size of the dynamic array by touching the last element
	 (*datalibrary_->unsolved_matrix_dyn_)(global_size-1,global_size-1)=0;
	 
	 for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		 if(datalibrary_->all_layers_info_[layer]->layer_used_){
			 for (uint32_t y_itor=0;y_itor<get_dyn_array(layer).nrows();y_itor++)
				 for (uint32_t x_itor=0;x_itor<get_dyn_array(layer).ncols();x_itor++) {
					 if (get_dyn_array(layer)[x_itor][y_itor].defined_==false)
						 continue;   //skip if unit is not defined
					 
					 create_unsolved_matrix_row(y_itor_global, layer, x_itor, y_itor);   //each model unit will represent one row
					 y_itor_global++;
					 
				 }
		 }	
			 //create row for T_inf
			 (*datalibrary_->unsolved_matrix_dyn_)(datalibrary_->unsolved_matrix_dyn_->nrows()-1,datalibrary_->unsolved_matrix_dyn_->ncols()-1)=1; //T_inf=T_ambient
			 
#ifdef _SESCTHERM_DEBUG
			 print_unsolved_model();
			 
#endif
			 //store the unsolved_matrix_dyn_ to the superLU matrix format (SuperMatrix at datalibrary_->A)
			 createSuperLUMatrix();
 }
 
 void sesctherm3Dmodel::store_non_zero(int x_coord, int32_t y_coord, double& value){
	 if(!EQ(value, 0.0))
		 (*datalibrary_->unsolved_matrix_dyn_)(x_coord, y_coord)=value;
 }
 
 //FIXME: if we have two ucool layers or >2 chip layers, there are changes that are going to have to be made here
 // In particular, there will be multiple virtual interface layers. Hence, Tvil will have to become Tvil[0],Tvil[1] etc etc
 void sesctherm3Dmodel::create_unsolved_matrix_row(int y_itor_global, int32_t layer, int32_t x_itor, int32_t y_itor) {
	 //Check: ensure that if two adjacent layers are of same dimension (in either x or y direction), that the offset is zero
	 int32_t layer_above=MIN(layer+1,(int)datalibrary_->all_layers_info_.size()-1);
	 int32_t layer_below=MAX(layer-1,0);
	 
	 
	 dynamic_array<model_unit>& temp_floorplan_dyn=get_dyn_array(layer);
	 // Tm,n,o
	 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor,y_itor), y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno);
	 // Tm-1,n,o
	 if(!EQ(temp_floorplan_dyn[x_itor][y_itor].x1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].x1_,0.0))
		 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor-1,y_itor),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_m_M1_no);
	 // Tm+1,n,o
	 if(!EQ(temp_floorplan_dyn[x_itor][y_itor].x3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].x3_,0.0))
		 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor+1,y_itor), y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_m_P1_no);
	 // Tm,n+1,o
	 if (!EQ(temp_floorplan_dyn[x_itor][y_itor].y3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].y3_,0.0)) {
		 //skip the UCOOL layer
		 if(datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER){
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer+2,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+2,datalibrary_);
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer+2, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_P1_o);
		 }
		 else{
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer+1,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			 
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer+1, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_P1_o);
		 }
		 
	 }
	 // Tm,n-1,o
	 if (!EQ(temp_floorplan_dyn[x_itor][y_itor].y1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].y1_,0.0)) {
		 if(layer==0){
			 cerr << "FATAL: sesctherm3Dmodel::create_unsolved_matrix_row: Tm,n-1,o MUST be 0 on Layer"<< layer << "(and it's not!)" << endl;
			 exit(1);
		 }
		 //skip the UCOOL layer
		 if(datalibrary_->all_layers_info_[layer-1]->type_==UCOOL_LAYER){
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer-2,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-2,datalibrary_);
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer-2, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_M1_o);
		 }
		 else{
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer-1,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			 
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer-1, x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mn_M1_o);
		 }	
		 
	 }
	 // Tm,n,o-1
	 if (!EQ(temp_floorplan_dyn[x_itor][y_itor].z1_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].z1_,0.0)) 
		 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor,y_itor-1),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno_M1);
	 // Tm,n,o+1
	 if (!EQ(temp_floorplan_dyn[x_itor][y_itor].z3_,-1.0) && !EQ(temp_floorplan_dyn[x_itor][y_itor].z3_,0.0)) 
		 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor,y_itor+1),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].t_mno_P1);
	 // Tvil,m,o
	 if (!EQ(temp_floorplan_dyn[x_itor][y_itor].tVIL_mo,0.0)) {
		 
		 if ( !(datalibrary_->all_layers_info_[layer]->type_==UCOOL_LAYER || datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER || datalibrary_->all_layers_info_[layer-1]->type_==UCOOL_LAYER) ) {
			 cerr << "FATAL: sesctherm3Dmodel::create_unsolved_matrix_row: ERROR!: There tVIL_mo MUST be 0 on Layer"<< layer << "(and it's not!)" << endl;
			 exit(1);
		 }
		 
		 
		 if( datalibrary_->all_layers_info_[layer+1]->type_==UCOOL_LAYER){
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer+1,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer+1,datalibrary_);
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer+1,x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo);
			 
		 }else if(datalibrary_->all_layers_info_[layer]->type_==UCOOL_LAYER){
			 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor, y_itor),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo);		
		 }
		 else{	//this means that datalibrary_->all_layers_info_[layer-1]==UCOOL_LAYER
			 int32_t x_itor_tmp=model_unit::find_model_unit_xitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].leftx_,layer-1,datalibrary_);
			 int32_t y_itor_tmp=model_unit::find_model_unit_yitor(get_dyn_array(layer)[x_itor][y_itor].leftx_,get_dyn_array(layer)[x_itor][y_itor].bottomy_,layer-1,datalibrary_);
			 
			 store_non_zero(find_unsolved_matrix_row_index(layer,x_itor_tmp, y_itor_tmp),y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tVIL_mo);
		 }
	 }
	 // T_inf m,n,o (this is the last element)
	 store_non_zero(datalibrary_->unsolved_matrix_dyn_->ncols()-1,y_itor_global,temp_floorplan_dyn[x_itor][y_itor].tINF_mno);
 }
 
 */

