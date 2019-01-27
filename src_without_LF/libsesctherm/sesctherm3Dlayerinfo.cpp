#include "sesctherm3Dinclude.h"
#include <math.h>

sesctherm3Dlayerinfo::sesctherm3Dlayerinfo(sesctherm3Ddatalibrary* datalibrary, int32_t num_layers){
	floorplan_layer_dyn_=new dynamic_array<model_unit>(datalibrary_);
	std::vector<model_unit*> located_units_layer;
	located_units_layer.clear();
	num_layers_=num_layers;
	for(int i=0;i<num_layers;i++){
		located_units_.push_back(located_units_layer);
	}
    layer_number_=-1;
	name_="-1";
	type_=-1;		
	thickness_=-1.0;
	height_=-1.0;
	width_=-1.0;
	leftx_=-1.0;		
	bottomy_=-1.0;	
	rightx_-1.0;
	topy_=-1.0;
	horizontal_conductivity_=-1.0;
	vertical_conductivity_down_=-1.0;
	vertical_conductivity_up_=-1.0;
	spec_heat_=-1.0;
	density_=-1.0;
	convection_coefficient_=-1.0;		
	granularity_=1.0;
	chip_floorplan_=NULL;	
	ucool_floorplan_=NULL;	
	datalibrary_=datalibrary;			
	unused_dyn_count_=0;
	layer_used_=true;
	temp_locking_enabled_=false;
	lock_temp_=-1.0;
}


sesctherm3Dlayerinfo::~sesctherm3Dlayerinfo(){
	//FIXME: this causes memory problems
	//		delete floorplan_layer_dyn_;
}


void sesctherm3Dlayerinfo::print(bool detailed){
	
	cerr << "Printing Layer Number [" << layer_number_ << "]" << endl;
	cerr << "Enabled? =" << layer_used_ << endl;
	cerr << "[name_] =" << name_ << endl;
	cerr << "[type_] =" << type_ << endl;
	cerr << "[thickness_] =" << thickness_ << endl;
	
	cerr << "[height_]=" << height_ << endl;
	cerr << "[width_]=" << width_ << endl;
	cerr << "[leftx_]=" << leftx_ << endl;		//calculated 
	cerr << "[bottomy_]=" << bottomy_ << endl;	//calculated
	cerr << "[rightx_]=" << rightx_ << endl;
	cerr << "[topy_]=" << topy_ << endl;
	
	//these are the effective material properties for the layer (averaged)

	cerr << "[horizontal_conductivity_]=" << horizontal_conductivity_ << endl;
	cerr << "[vertical_conductivity_down_]=" << vertical_conductivity_down_ << endl;
	cerr << "[vertical_conductivity_up_]=" << vertical_conductivity_up_ << endl;
	cerr << "[spec_heat_]=" << spec_heat_ << endl;
	cerr << "[density_]=" << density_ << endl;
	cerr << "[convection_coefficient_]=" << convection_coefficient_ << endl;		//used for fluid/air layers
	cerr << "[granularity_]=" << granularity_ << endl;
	cerr << "[flp_num_]=" << flp_num_ << endl;
	//cerr << "[heat transfer methods]=" << std::hex << heat_transfer_methods_ << endl;
	
	
	if(datalibrary_ == NULL){
		cerr << "FATAL: sesctherm3Dlayerinfo::operator<< => datalibrary_ == NULL" << endl;
		exit(1);
	}
	
	dynamic_array<model_unit>* floorplan_layer_dyn_;	//this is the dynamic array with corresponds to this layer
	int unused_dyn_count_;	
	
	if(chip_floorplan_!=NULL){
		cerr << endl;
		cerr << *chip_floorplan_;			//print chip floorplan
		cerr << endl;
	}
	if(ucool_floorplan_!=NULL){
		cerr << endl;
		cerr << *ucool_floorplan_;			//print ucool_floorplan
		cerr << endl;
	}
	
	if(detailed){
		//print the dynamic array that corresponds to this layer
		model_unit::print_dyn_layer(layer_number_, datalibrary_);
		
	}
	cerr << endl << endl;
}



void sesctherm3Dlayerinfo::print_lumped_metrics(){
	double lumped_vertical_resistance_above=0;
	double lumped_vertical_resistance_below=0;
	double lumped_ambient_resistance=0;
	double lumped_lateral_resistance=0;
	double lumped_capacitance=0;
	double lumped_rc_layer_above=0;
	double lumped_rc_layer_below=0;
	double lumped_rc_ambient=0;
	double lumped_rc_within_layer=0;
	
	int y_itor_global=0;
	for (uint32_t layer=0;layer<datalibrary_->all_layers_info_.size();layer++)
		if(datalibrary_->all_layers_info_[layer]->layer_used_){
			for (uint32_t y_itor=0;y_itor<floorplan_layer_dyn_->nrows();y_itor++)
				for (uint32_t x_itor=0;x_itor<floorplan_layer_dyn_->ncols();x_itor++) {
					model_unit& munit=(*floorplan_layer_dyn_)[x_itor][y_itor];
					if (munit.defined_==false)
						continue;   //skip if unit is not defined
				
					if(munit.t_mn_P1_o!=0){
						lumped_vertical_resistance_above+=(*munit.t_mn_P1_o)*-1.0;
					}
					
					if(munit.t_mn_M1_o!=0){
						lumped_vertical_resistance_below+=(*munit.t_mn_M1_o)*-1.0;
					}
					
					if(munit.tINF_mno!=0){
						lumped_ambient_resistance+=(*munit.tINF_mno)*-1.0;
					}
					double tmp_lateral_resistance=0;
					if(munit.t_m_M1_no!=0)
						tmp_lateral_resistance+=*munit.t_m_M1_no*-1.0;
					if(munit.t_m_P1_no!=0)
						tmp_lateral_resistance+=*munit.t_m_P1_no*-1.0;
					if(munit.t_mno_M1!=0)
						tmp_lateral_resistance+=*munit.t_mno_M1*-1.0;
					if(munit.t_mno_P1!=0)
						tmp_lateral_resistance+=*munit.t_mno_P1*-1.0;
					lumped_lateral_resistance+=tmp_lateral_resistance/4;
					
					lumped_capacitance+=munit.specific_heat_*munit.x2_*munit.y2_*munit.z2_*munit.row_;
				}
		}	
			if(lumped_vertical_resistance_above!=0)
				lumped_vertical_resistance_above=(1/lumped_vertical_resistance_above);
			else
				lumped_vertical_resistance_above=0;
			
			if(lumped_vertical_resistance_below!=0)
				lumped_vertical_resistance_below=(1/lumped_vertical_resistance_below);
			else 
				lumped_vertical_resistance_below=0;
			
			if(lumped_ambient_resistance!=0)
				lumped_ambient_resistance=(1/lumped_ambient_resistance);
			else
				lumped_ambient_resistance=0;
			
			if(lumped_lateral_resistance!=0){
				lumped_lateral_resistance=(1/lumped_lateral_resistance);
			}
			else{
				lumped_lateral_resistance=0;
			}
			
			lumped_rc_layer_above=lumped_vertical_resistance_above*lumped_capacitance;
			lumped_rc_layer_below=lumped_vertical_resistance_below*lumped_capacitance;
			lumped_rc_ambient=lumped_rc_ambient*lumped_capacitance;
			lumped_rc_within_layer=lumped_lateral_resistance*lumped_capacitance;
			
	
			cerr << "Printing Layer Number  [" << layer_number_ << "] Lumped Metric Data" << endl;
			cerr << "[Lumped Vertical Resistance to layer above (K/W):]" << lumped_vertical_resistance_above  << endl;
			cerr << "[Lumped Vertical Resistance to layer below (K/W):]" << lumped_vertical_resistance_below  << endl;
			cerr << "[Lumped Resistance to ambient (K/W):]" << lumped_ambient_resistance  << endl;
			cerr << "[Lumped Laterial Resistance within layer (K/W):]" << lumped_lateral_resistance  << endl;
			cerr << "[Lumped Capacitance (W/K):]" << lumped_capacitance  << endl;
			cerr << "[Lumped RC-time constant to layer above (s):]" << lumped_rc_layer_above  << endl;
			cerr << "[Lumped RC-time constant to layer below (s):]" << lumped_rc_layer_below  << endl;
			cerr << "[Lumped RC-time constant to layer ambient (s):]" << lumped_rc_ambient  << endl;
			cerr << "[Lumped RC-time constant within layer (s):]" << lumped_rc_within_layer << endl;
			cerr << endl;
	
}



//1) computes the absolute coordinates for the various layers
//2) determines the amount of space necessary for each layer, then allocates the space 
void sesctherm3Dlayerinfo::allocate_layers(sesctherm3Ddatalibrary* datalibrary_){
	//generate the leftx, rightx, bottomy, and topy coordinates for each layer
	//first we find the widest and tallest layers (this is used to generate the the absolute coordinates)
	//the algorithm simple centers each layer with respect to the model width and height
	double model_width=0;
	double model_height=0;
	std::vector<temporary_layer_info> layerinfo;		//granularities to index

	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		model_width=MAX(model_width,datalibrary_->all_layers_info_[i]->width_);
		model_height=MAX(model_height,datalibrary_->all_layers_info_[i]->height_);
	}
	
	
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		datalibrary_->all_layers_info_[i]->leftx_=(model_width/2)-(datalibrary_->all_layers_info_[i]->width_/2);
		datalibrary_->all_layers_info_[i]->rightx_=datalibrary_->all_layers_info_[i]->leftx_+datalibrary_->all_layers_info_[i]->width_;
		
		datalibrary_->all_layers_info_[i]->bottomy_=(model_height/2)-(datalibrary_->all_layers_info_[i]->height_/2);
		datalibrary_->all_layers_info_[i]->topy_=datalibrary_->all_layers_info_[i]->bottomy_+datalibrary_->all_layers_info_[i]->height_;

	}

	//Now fill in all the missing granularities
	//If a particular layer's granularity is defined as -1, then it should be the same the finest granularity of the adjacent layers
	//if the granularity is zero, this means that the granularity is effectively infinity (coarsest granularity)
	
	for(int j=0;j<(int)datalibrary_->all_layers_info_.size();j++){
		if(EQ(datalibrary_->all_layers_info_[j]->granularity_,0.0))
			datalibrary_->all_layers_info_[j]->granularity_=pow(10,5.0);	//set the granularities to infinity
		else if(EQ(datalibrary_->all_layers_info_[j]->granularity_,-1.0))
			datalibrary_->all_layers_info_[j]->granularity_=pow(10,6.0);	//set the granularities to minimum of adjacent
	}		

	
	//This requires an O(N^2) operation where we propagate the granularities to the missing layers
	for(int j=0;j<(int)datalibrary_->all_layers_info_.size();j++){
		for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
			int layer_below=MAX(0,i-1);
			int layer_above=MIN((int)datalibrary_->all_layers_info_.size()-1,i+1);
			double min=MIN(datalibrary_->all_layers_info_[layer_below]->granularity_, 
						   datalibrary_->all_layers_info_[layer_above]->granularity_);
			if(EQ(datalibrary_->all_layers_info_[j]->granularity_,pow(10,6.0)) && LT(min, pow(10,5.0)))
				datalibrary_->all_layers_info_[i]->granularity_=min;

			
		}
	}
	
	for(int j=0;j<(int)datalibrary_->all_layers_info_.size();j++){
		if(EQ(datalibrary_->all_layers_info_[j]->granularity_,pow(10,5.0)))
			datalibrary_->all_layers_info_[j]->granularity_=99999999;	//set the granularities to infinity
	}		
	
	//make a guess as to the size of the dynamic arrays for each layer
	int flp_count=0;
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		temporary_layer_info newinfo(i,
									datalibrary_->all_layers_info_[i]->granularity_,
									datalibrary_->all_layers_info_[i]->height_,
									datalibrary_->all_layers_info_[i]->width_,
									datalibrary_->all_layers_info_[i]->height_*datalibrary_->all_layers_info_[i]->width_,
									0,
									datalibrary_->all_layers_info_[i]->height_*datalibrary_->all_layers_info_[i]->width_,
									0
									 );
		layerinfo.push_back(newinfo);
		if(datalibrary_->all_layers_info_[i]->chip_floorplan_!=NULL)
			flp_count+=datalibrary_->all_layers_info_[layerinfo[i].layer_num_]->chip_floorplan_->flp_units_.size();

	}	
	
	
	//sort by granularity and then size
	sort(layerinfo.begin(), layerinfo.end(), temporary_layer_info::cmpGranularityThenSize());
	
	uint32_t i;
/*
	for( i = 0; i < layerinfo.size(); i++){
		cerr << i << "\t" << layerinfo[i].layer_num_ << "\t" << layerinfo[i].granularity_ << "\t" << layerinfo[i].height_ << "\t" << layerinfo[i].width_ << "\t" << layerinfo[i].size_ << endl;
	}
*/	
	
	for(i = 0; i != layerinfo.size(); i++){
		layerinfo[i].nregions_+=(int)(layerinfo[i].areas_/(powf(layerinfo[i].granularity_, 2.0))) + flp_count;
		
		uint32_t j= i;
		j++;
		for(; j < layerinfo.size();j++){

			double width=MIN(datalibrary_->all_layers_info_[layerinfo[j].layer_num_]->rightx_,
							 datalibrary_->all_layers_info_[layerinfo[i].layer_num_]->rightx_)
				-
				MAX(datalibrary_->all_layers_info_[layerinfo[j].layer_num_]->leftx_,
					datalibrary_->all_layers_info_[layerinfo[i].layer_num_]->leftx_); 
			;
			double height=MIN(datalibrary_->all_layers_info_[layerinfo[j].layer_num_]->topy_,
							  datalibrary_->all_layers_info_[layerinfo[i].layer_num_]->topy_) 
				-
				MAX(datalibrary_->all_layers_info_[layerinfo[j].layer_num_]->bottomy_,
					datalibrary_->all_layers_info_[layerinfo[i].layer_num_]->bottomy_);
			layerinfo[j].nregions_+=(height*width - layerinfo[j].areas_removed_)/(powf(layerinfo[i].granularity_,2.0));
			layerinfo[j].areas_-=width*height;
			layerinfo[j].areas_removed_+=width*height;
		}
	}
	
	for(int i=0;i<(int)datalibrary_->all_layers_info_.size();i++){
		datalibrary_->all_layers_info_[i]->floorplan_layer_dyn_ = new dynamic_array <model_unit> ((int)sqrt(layerinfo[i].nregions_), (int)sqrt(layerinfo[i].nregions_), datalibrary_);
	}
}


void sesctherm3Dlayerinfo::offset_layer(){
	switch(type_){
		case DIE_TRANSISTOR_LAYER:
			if(chip_floorplan_==NULL){
				cerr << "FATAL: sesctherm3Dlayerinfo::offset_layer => DIE_TRANSISTOR_LAYER must have an associated floorplan defined (currently set to -1)" << endl;
				exit(1);
			}
			break;
		case UCOOL_LAYER:
			if(ucool_floorplan_==NULL){
				cerr << "FATAL: sesctherm3Dlayerinfo::offset_layer => UCOOL_LAYER must have an associated floorplan defined (currently set to -1)" << endl;
				exit(1);
			}
			ucool_floorplan_->offset_floorplan(leftx_,bottomy_);
			break;
	}
	
	if(type_!=UCOOL_LAYER && chip_floorplan_!=NULL)
		chip_floorplan_->offset_floorplan(leftx_,bottomy_);
}

void sesctherm3Dlayerinfo::determine_layer_properties(){
	switch(type_){
		case MAINBOARD_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}
			sesctherm3Dmaterial::calc_pcb_layer_properties(*this, datalibrary_);
#ifdef _SESCTHERM_DEBUG
			cerr << "Built mainboard_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;			
		case PINS_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_pins_layer_properties(*this, datalibrary_);
#ifdef _SESCTHERM_DEBUG
			cerr << "Built pins_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case PWB_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_pwb_layer_properties(*this,datalibrary_);	
#ifdef _SESCTHERM_DEBUG
			cerr << "Built pwb_layer for layer number [" << layer_number_ << "]" << endl;
#endif	
			break;
		case FCPBGA_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_package_substrate_layer_properties(*this,datalibrary_);		
#ifdef _SESCTHERM_DEBUG
			cerr << "Built fcpbga_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case C4_UNDERFILL_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_c4underfill_layer_properties(*this,datalibrary_);				
#ifdef _SESCTHERM_DEBUG
			cerr << "Built c4_underfill_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case INTERCONNECT_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_interconnect_layer_properties(*this,datalibrary_);	
#ifdef _SESCTHERM_DEBUG
			cerr << "Built interconnect_layer for layer number [" << layer_number_ << "]" << endl;
#endif	
			break;
		case DIE_TRANSISTOR_LAYER:
			if(flp_num_==-1){
				cerr << "FATAL: sesctherm3Dlayer::determine_layer_properties => floorplan value must not be equal to -1 for transistor layer" << endl;
				exit(1);
			}
				
			chip_floorplan_= new chip_floorplan(datalibrary_);
			chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			
			//Offset the datapoints of the chip floorplan to reflect the relative size of the heat_sink
			sesctherm3Dmaterial::calc_die_transistor_layer_properties(*this,datalibrary_);		
			
#ifdef _SESCTHERM_DEBUG
			cerr << "Built chip floorplan for layer number [" << layer_number_ << "]" << endl;
#endif
			
			
			break;
		case BULK_SI_LAYER:
			if(flp_num_!=-1){
				chip_floorplan_= new chip_floorplan(datalibrary_);
				chip_floorplan_->get_floorplan(datalibrary_->config_data_->get_chipflp_from_sescconf_, flp_num_);
			}			
			sesctherm3Dmaterial::calc_silicon_layer_properties(*this,datalibrary_);		
#ifdef _SESCTHERM_DEBUG
			cerr << "Built builk_si_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case OIL_LAYER:			
			sesctherm3Dmaterial::calc_oil_layer_properties(*this,datalibrary_);	
#ifdef _SESCTHERM_DEBUG
			cerr << "Built oil_layer for layer number [" << layer_number_ << "]" << endl;
#endif	
			break;
		case AIR_LAYER:			
			sesctherm3Dmaterial::calc_air_layer_properties(*this,datalibrary_);	
#ifdef _SESCTHERM_DEBUG
			cerr << "Built air_layer for layer number [" << layer_number_ << "]" << endl;
#endif	
			break;			
		case UCOOL_LAYER:
			//open the ucooler floorplan file, store the information to the ucooler_floorplan structure
			ucool_floorplan_ = new ucool_floorplan(datalibrary_);
			ucool_floorplan_->get_floorplan(datalibrary_->if_ucool_flpfile_, 
											datalibrary_->config_data_->ucool_width_, 
											datalibrary_->config_data_->ucool_height_, 
											datalibrary_->config_data_->get_chipflp_from_sescconf_,
											datalibrary_->config_data_->generate_ucool_floorplan_);
			//Offset the datapoints of the ucooler floorplan to reflect the relative size of the heat_sink
			
			
			//sesctherm3Dmaterial::calc_ucool_layer_properties(*this,datalibrary_);	
#ifdef _SESCTHERM_DEBUG
			cerr << "Built ucool floorplan for layer number [" << layer_number_ << "]" << endl;
#endif	
			break;
		case HEAT_SPREADER_LAYER:
			sesctherm3Dmaterial::calc_heat_spreader_layer_properties(*this,datalibrary_);		
#ifdef _SESCTHERM_DEBUG
			cerr << "Built heat_spreader_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case HEAT_SINK_LAYER:
			sesctherm3Dmaterial::calc_heat_sink_layer_properties(*this,datalibrary_);		
#ifdef _SESCTHERM_DEBUG
			cerr << "Built heat_sink_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		case HEAT_SINK_FINS_LAYER:
			sesctherm3Dmaterial::calc_heat_sink_fins_layer_properties(*this,datalibrary_);		
#ifdef _SESCTHERM_DEBUG
			cerr << "Built heat_sink_fins_layer for layer number [" << layer_number_ << "]" << endl;
#endif
			break;
		default: 
			cerr<< "FATAL: sesctherm3Dlayerinfo::determine_layer_properties -- undefined layer type [" << type_ << "]" << endl;
			exit(1);
	}
}

//this returns an array which contains the averaged temperature value of the model units that correspond to each floorplan unit
//ex: model units 1,4,9 may be the model units contained within the floorplan unit zero, therefore the 0th element of the returned
//array will contain the averaged temperature values of those model units
std::vector<double> sesctherm3Dlayerinfo::compute_average_temps(int flp_layer) {
	
	if(datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_ == NULL) {
		cerr << "FATAL: sesctherm3Dlayerinfo::computer_average_temps() => flp_layer [" << layer_number_ << "] has undefined chip_floorplan_!" << endl;
		exit(1);
	}
	
	std::vector<double> temperature_map;
	double average_temp=0;
	for(int i=0;i<(int)datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_.size();i++){
		chip_flp_unit& flp_unit=datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[i];
		
		if (flp_unit.located_units_[layer_number_].empty()) {
			model_unit::locate_model_units(layer_number_,
										   flp_unit.located_units_[layer_number_],
										   flp_unit.leftx_,
										   flp_unit.bottomy_,
										   flp_unit.leftx_+flp_unit.width_,
										   flp_unit.bottomy_+flp_unit.height_,
										   datalibrary_);
		}
		average_temp=0;
		
		
#ifdef _SESCTHERM_DEBUG
//		cout << "[FLPUNIT:" << i << "][layer" << layer_number_ << "][located_units:" << flp_unit.located_units_[layer_number_].size() << " ]" << endl;
#endif		
		
		
		//compute average temperature for the units
		for(int j=0;j<(int)flp_unit.located_units_[layer_number_].size();j++){
			if(flp_unit.located_units_[layer_number_][j]->get_temperature()!=NULL)
				average_temp+=*(flp_unit.located_units_[layer_number_][j]->get_temperature());
			else
				average_temp+=0;
		}									
		average_temp/=flp_unit.located_units_[layer_number_].size();
		temperature_map.push_back(average_temp);	
	}
	return(temperature_map);
}

std::vector<double> sesctherm3Dlayerinfo::compute_average_powers(int flp_layer) {
	
	std::vector<double> power_map;
	if(datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_ == NULL) {
		cerr << "FATAL: sesctherm3Dlayerinfo::computer_average_powers() => flp_layer [" << layer_number_ << "] has undefined chip_floorplan_!" << endl;
		exit(1);
	}
	
	power_map_.clear();
	double average_temp=0;
	for(int i=0;i<(int)datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_.size();i++){
		chip_flp_unit& flp_unit=datalibrary_->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[i];
		if (flp_unit.located_units_[layer_number_].empty()) {
			model_unit::locate_model_units(layer_number_,
										   flp_unit.located_units_[layer_number_],
										   flp_unit.leftx_,
										   flp_unit.bottomy_,
										   flp_unit.leftx_+flp_unit.width_,
										   flp_unit.bottomy_+flp_unit.height_,
										   datalibrary_);
		}
		average_temp=0;
		//compute average temperature for the units
		for(int j=0;j<(int)flp_unit.located_units_[layer_number_].size();j++){
			if(flp_unit.located_units_[layer_number_][j]->get_temperature()!=NULL)
				average_temp+=*(flp_unit.located_units_[layer_number_][j]->get_temperature());
			else
				average_temp+=0;
		}									
		average_temp/=flp_unit.located_units_[layer_number_].size();
		power_map.push_back(average_temp);	
	}
	return(power_map);
}

//this computes average temperatures and powers across all of the layers
dynamic_array<model_unit> sesctherm3Dlayerinfo::compute_layer_averages(std::vector<int>& layers, sesctherm3Ddatalibrary* datalibrary_){
	dynamic_array<model_unit> temp_layer_dyn(datalibrary_);
	int	   biggest_layer_num=0;
	double max_x=-1.0;
	double max_y=-1.0;
	for(uint32_t i=0;i<layers.size();i++){
		int layer_num=layers[i];
		//Note: we assume that there will ALWAYS be a large layer than encloses the others
		if(datalibrary_->all_layers_info_[layer_num]->height_ > max_y || datalibrary_->all_layers_info_[layer_num]->width_ > max_x){
			max_x=datalibrary_->all_layers_info_[layer_num]->width_;
			max_y=datalibrary_->all_layers_info_[layer_num]->height_;
			biggest_layer_num=layer_num; 
		}
	}
	//copy the largest layer to temp_layer_dyn
	for(uint32_t itor_y=0;itor_y<datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_->nrows();itor_y++){
		for(uint32_t itor_x=0;itor_x<datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_->ncols();itor_x++){
			temp_layer_dyn[itor_x][itor_y]=(*datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_)[itor_x][itor_y];
		}
	}
	
	//go through the temp_layer_dyn and compute the average over all the relevent layers
	for(uint32_t itor_y=0;itor_y<temp_layer_dyn.nrows();itor_y++)
		for(uint32_t itor_x=0;itor_x<temp_layer_dyn.ncols();itor_x++){
			//set averages to zero
			temp_layer_dyn[itor_x][itor_y].energy_data_=0;
			temp_layer_dyn[itor_x][itor_y].sample_temperature_=0;
			for(uint32_t i=0;i<layers.size();i++){
				int layer_num=layers[i];
				temp_layer_dyn[itor_x][itor_y].energy_data_+=(model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
																						  temp_layer_dyn[itor_x][itor_y].bottomy_,
																						  layer_num,
																						  datalibrary_))->energy_data_;
				SUElement_t* temperature_ptr=(model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
										     temp_layer_dyn[itor_x][itor_y].bottomy_,
											 layer_num,
										     datalibrary_))->get_temperature();
				if(temperature_ptr!=NULL)
					temp_layer_dyn[itor_x][itor_y].sample_temperature_+=*temperature_ptr;
			}
			temp_layer_dyn[itor_x][itor_y].energy_data_/=(layers.size());
			temp_layer_dyn[itor_x][itor_y].sample_temperature_/=(layers.size());
		}
			return(temp_layer_dyn);	
}

//this computes the difference in power and temperature between two layers
dynamic_array<model_unit> sesctherm3Dlayerinfo::compute_layer_diff(int layer1, int32_t layer2, sesctherm3Ddatalibrary* datalibrary_){
	dynamic_array<model_unit> temp_layer_dyn(datalibrary_);
	int	   biggest_layer_num=0;
	//Note: we assume that there will ALWAYS be a large layer than encloses the others
	if(datalibrary_->all_layers_info_[layer1]->height_ > datalibrary_->all_layers_info_[layer2]->height_ 
	   || datalibrary_->all_layers_info_[layer1]->width_ > datalibrary_->all_layers_info_[layer2]->width_){
		biggest_layer_num=layer1; 
	}
	else{
		biggest_layer_num=layer2;
		
		//copy the largest layer to temp_layer_dyn
		for(uint32_t itor_y=0;itor_y<datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_->nrows();itor_y++){
			for(uint32_t itor_x=0;itor_x<datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_->ncols();itor_x++){
				temp_layer_dyn[itor_x][itor_y]=(*datalibrary_->all_layers_info_[biggest_layer_num]->floorplan_layer_dyn_)[itor_x][itor_y];
			}
		}
		
		//go through the temp_layer_dyn and compute the absolute difference between the two layers
		for(uint32_t itor_y=0;itor_y<temp_layer_dyn.nrows();itor_y++)
			for(uint32_t itor_x=0;itor_x<temp_layer_dyn.ncols();itor_x++){
				//set averages to zero
				temp_layer_dyn[itor_x][itor_y].energy_data_=0;
				temp_layer_dyn[itor_x][itor_y].sample_temperature_=0;
				
				temp_layer_dyn[itor_x][itor_y].energy_data_=ABS_DIF(model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
																								temp_layer_dyn[itor_x][itor_y].bottomy_,
																								layer1,
																								datalibrary_)->energy_data_,
																	model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
																								temp_layer_dyn[itor_x][itor_y].bottomy_,
																								layer2,
																								datalibrary_)->energy_data_);
				SUElement_t* temperature_ptr1=model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
																	 temp_layer_dyn[itor_x][itor_y].bottomy_,
																	 layer1,
																	 datalibrary_)->get_temperature();
				SUElement_t* temperature_ptr2=model_unit::find_model_unit(temp_layer_dyn[itor_x][itor_y].leftx_, 
																	 temp_layer_dyn[itor_x][itor_y].bottomy_,
																	 layer2,
																	 datalibrary_)->get_temperature();
				double temp1, temp2;
				if(temperature_ptr1!=NULL)
					temp1=*temperature_ptr1;
				else
					temp1=0;
				
				if(temperature_ptr2!=NULL)
					temp2=*temperature_ptr2;
				else
					temp2=0;
				
				temp_layer_dyn[itor_x][itor_y].sample_temperature_=ABS_DIF(temp1,temp2);
			}
	}
		return(temp_layer_dyn);	
}


void sesctherm3Dlayerinfo::accumulate_data_for_sample(){
	dynamic_array<model_unit>& dyn_array = *floorplan_layer_dyn_;
	for (uint32_t y_itor=0;y_itor<dyn_array.nrows();y_itor++)
		for (uint32_t x_itor=0;x_itor<dyn_array.ncols();x_itor++){ 
			if(!dyn_array[x_itor][y_itor].defined_)
				continue;
				running_average_temperature_map_[x_itor][y_itor]+=*dyn_array[x_itor][y_itor].get_temperature()*datalibrary_->timestep_;
				running_average_energy_map_[x_itor][y_itor]+=dyn_array[x_itor][y_itor].energy_data_*datalibrary_->timestep_;
				running_max_temperature_map_[x_itor][y_itor]=MAX(running_max_temperature_map_[x_itor][y_itor], 	*dyn_array[x_itor][y_itor].get_temperature());
				running_min_temperature_map_[x_itor][y_itor]=MIN(running_min_temperature_map_[x_itor][y_itor], 	*dyn_array[x_itor][y_itor].get_temperature());
				running_max_energy_map_[x_itor][y_itor]=MAX(running_max_energy_map_[x_itor][y_itor], dyn_array[x_itor][y_itor].energy_data_);
				running_min_energy_map_[x_itor][y_itor]=MIN(running_min_energy_map_[x_itor][y_itor], dyn_array[x_itor][y_itor].energy_data_);
		}
}

//this computes a running average of each base_unit over each of the graphics durations 
void sesctherm3Dlayerinfo::compute_sample(){
	dynamic_array<model_unit>& dyn_array= *floorplan_layer_dyn_;
	for (uint32_t y_itor=0;y_itor<dyn_array.nrows();y_itor++)
		for (uint32_t x_itor=0;x_itor<dyn_array.ncols();x_itor++){ 
			if(!dyn_array[x_itor][y_itor].defined_)
				continue;
			//store the average temperature
			dyn_array[x_itor][y_itor].ave_temperature_=running_average_temperature_map_[x_itor][y_itor]/datalibrary_->config_data_->sample_duration_;
			//clear the running average
			running_average_temperature_map_[x_itor][y_itor]=0;
			//store the average energy
			dyn_array[x_itor][y_itor].ave_energy_data_=running_average_energy_map_[x_itor][y_itor]/datalibrary_->config_data_->sample_duration_;
			//clear the running average
			running_average_energy_map_[x_itor][y_itor]=0;
			//store the max temperature
			dyn_array[x_itor][y_itor].max_temperature_=running_max_temperature_map_[x_itor][y_itor];
			//clear the running max
			running_max_temperature_map_[x_itor][y_itor]=0;
			//store the min temperature
			dyn_array[x_itor][y_itor].min_temperature_=running_min_temperature_map_[x_itor][y_itor];
			//clear the running min
			running_min_temperature_map_[x_itor][y_itor]=powf(10.0,10);
			//store the max energy
			dyn_array[x_itor][y_itor].max_energy_data_=running_max_energy_map_[x_itor][y_itor];
			//clear the running max
			running_max_energy_map_[x_itor][y_itor]=0;
			//store the min energy
			dyn_array[x_itor][y_itor].min_energy_data_=running_min_energy_map_[x_itor][y_itor];
			//clear the running min
			running_min_energy_map_[x_itor][y_itor]=powf(10.0,10);
			
		}					
}


std::vector<double> sesctherm3Dlayerinfo::compute_energy(){
	I(chip_floorplan_!=NULL);
	std::vector<double> power_map;
	power_map.clear();
	for(int i=0;i<(int)chip_floorplan_->flp_units_.size();i++){
		power_map.push_back(chip_floorplan_->flp_units_[i].power_);
	}
	return (power_map);
}



/*
 //Note: these offsets are only useful if we assume that the regions are overlapping
 //If there is no overlap, these won't work
 void sesctherm3Dlayerinfo::compute_offsets(int layer_num, sesctherm3Ddatalibrary* datalibrary_){
	 double x_itor=0;
	 double y_itor=0;
	 double x_offset=0;
	 double y_offset=0;
	 
	 sesctherm3Dlayerinfo& layer = datalibrary_->all_layers_info_(layer_num);
	 
	 dynamic_array<model_unit>& dyn_array = layer.floorplan_layer_dyn_;
	 
	 //check to ensure layer number is in range
	 if(layer.layer_number_ < 0 || layer.layer_number_ > datalibrary_->all_layers_info_.size()-1){
		 cerr << "sesctherm3Dlayerinfo::compute_offsets => layer [" << layernum <<  "].layer_number_ is out of range" << endl;
		 exit(1);
	 }
	 
	 //if we are on the bottom layer, no offset below
	 if(layer.layer_number_ == 0){
		 sesctherm3Dlayerinfo& layer_above = datalibrary_->all_layers_info_[layer_num+1];
		 dynamic_array<model_unit>& dyn_array_above = layer_above.floorplan_layer_dyn_;
		 
		 
		 layer.offset_layer_below_y_=0;
		 layer.offset_layer_below_x_=0;
		 
		 
		 
		 //FIND OFFSET ABOVE X
		 x_itor=0;		
		 //if the left datapoint of the above layer is smaller than the current layer, then specify pcerritive offset
		 if(LT(layer_above.leftx_, layer.leftx_)){
			 while(!EQ(dyn_array_above[x_itor][0].leftx_, dyn_array[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_above_x_=x_itor;
		 }
		 x_itor=0;
		 //if the left datapoint of the above layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_above.leftx_, layer.leftx_)){	
			 while(!EQ(dyn_array[x_itor][0].leftx_, dyn_array_above[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_above_x_=-1*x_itor;
		 }
		 else 
			 layer.offset_layer_above_x_=0;
		 y_itor=0;
		 
		 
		 //FIND OFFSET ABOVE Y
		 //if the bottom datapoint of the above layer is smaller than the current layer, then specify pcerritive offset 
		 if(LT(layer_above.bottomy_, layer.bottomy_)){
			 while(!EQ(dyn_array_above[0][y_itor].bottomy_, dyn_array[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_above_y_=y_itor;
		 }
		 y_itor=0;
		 //if the bottom datapoint of the above layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_above.bottomy_, layer.bottomy_)){	
			 while(!EQ(dyn_array[0][y_itor].bottomy_, dyn_array_above[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_above_y_=-1*y_itor;
		 }
		 else 
			 layer.offset_layer_above_y_=0;
		 
	 }
	 else if(layer.layer_number == datalibrary_->all_layers_info_.size()-1){
		 sesctherm3Dlayerinfo& layer_above = datalibrary_->all_layers_info_[layer_num+1];
		 dynamic_array<model_unit>& dyn_array_above = layer_above.floorplan_layer_dyn_;
		 
		 layer.offset_layer_above_y_=0;
		 layer.offset_layer_above_x_=0;
		 
		 
		 //FIND OFFSET BELOW X
		 x_itor=0;		
		 //if the left datapoint of the below layer is smaller than the current layer, then specify pcerritive offset
		 if(LT(layer_below.leftx_, layer.leftx_)){
			 while(!EQ(dyn_array_below[x_itor][0].leftx_, dyn_array[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_below_x_=x_itor;
		 }
		 x_itor=0;
		 //if the left datapoint of the below layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_below.leftx_, layer.leftx_)){	
			 while(!EQ(dyn_array[x_itor][0].leftx_, dyn_array_below[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_below_x_=-1*x_itor;
		 }
		 else 
			 layer.offset_layer_below_x_=0;
		 y_itor=0;
		 
		 
		 //FIND OFFSET BELOW Y
		 //if the bottom datapoint of the below layer is smaller than the current layer, then specify pcerritive offset 
		 if(LT(layer_below.bottomy_, layer.bottomy_)){
			 while(!EQ(dyn_array_below[0][y_itor].bottomy_, dyn_array[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_below_y_=y_itor;
		 }
		 y_itor=0;
		 //if the bottom datapoint of the below layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_below.bottomy_, layer.bottomy_)){	
			 while(!EQ(dyn_array[0][y_itor].bottomy_, dyn_array_below[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_below_y_=-1*y_itor;
		 }
		 else 
			 layer.offset_layer_below_y_=0;
	 }
	 else{
		 sesctherm3Dlayerinfo& layer_above = datalibrary_->all_layers_info_[layer_num+1];
		 dynamic_array<model_unit>& dyn_array_above = layer_above.floorplan_layer_dyn_;
		 sesctherm3Dlayerinfo& layer_above = datalibrary_->all_layers_info_[layer_num+1];
		 dynamic_array<model_unit>& dyn_array_above = layer_above.floorplan_layer_dyn_;
		 
		 //FIND OFFSET ABOVE X
		 x_itor=0;		
		 //if the left datapoint of the above layer is smaller than the current layer, then specify pcerritive offset
		 if(LT(layer_above.leftx_, layer.leftx_)){
			 while(!EQ(dyn_array_above[x_itor][0].leftx_, dyn_array[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_above_x_=x_itor;
		 }
		 x_itor=0;
		 //if the left datapoint of the above layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_above.leftx_, layer.leftx_)){	
			 while(!EQ(dyn_array[x_itor][0].leftx_, dyn_array_above[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_above_x_=-1*x_itor;
		 }
		 else 
			 layer.offset_layer_above_x_=0;
		 y_itor=0;
		 
		 
		 //FIND OFFSET ABOVE Y
		 //if the bottom datapoint of the above layer is smaller than the current layer, then specify pcerritive offset 
		 if(LT(layer_above.bottomy_, layer.bottomy_)){
			 while(!EQ(dyn_array_above[0][y_itor].bottomy_, dyn_array[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_above_y_=y_itor;
		 }
		 y_itor=0;
		 //if the bottom datapoint of the above layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_above.bottomy_, layer.bottomy_)){	
			 while(!EQ(dyn_array[0][y_itor].bottomy_, dyn_array_above[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_above_y_=-1*y_itor;
		 }
		 else 
			 layer.offset_layer_above_y_=0;
		 
		 
		 //FIND OFFSET BELOW X
		 x_itor=0;		
		 //if the left datapoint of the below layer is smaller than the current layer, then specify pcerritive offset
		 if(LT(layer_below.leftx_, layer.leftx_)){
			 while(!EQ(dyn_array_below[x_itor][0].leftx_, dyn_array[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_below_x_=x_itor;
		 }
		 x_itor=0;
		 //if the left datapoint of the below layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_below.leftx_, layer.leftx_)){	
			 while(!EQ(dyn_array[x_itor][0].leftx_, dyn_array_below[0][0].leftx_)){
				 x_itor++;
			 }
			 layer.offset_layer_below_x_=-1*x_itor;
		 }
		 else 
			 layer.offset_layer_below_x_=0;
		 y_itor=0;
		 
		 
		 //FIND OFFSET BELOW Y
		 //if the bottom datapoint of the below layer is smaller than the current layer, then specify pcerritive offset 
		 if(LT(layer_below.bottomy_, layer.bottomy_)){
			 while(!EQ(dyn_array_below[0][y_itor].bottomy_, dyn_array[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_below_y_=y_itor;
		 }
		 y_itor=0;
		 //if the bottom datapoint of the below layer is bigger than the current layer, then specify negative offset
		 else if(GT(layer_below.bottomy_, layer.bottomy_)){	
			 while(!EQ(dyn_array[0][y_itor].bottomy_, dyn_array_below[0][0].bottomy_)){
				 y_itor++;
			 }
			 layer.offset_layer_below_y_=-1*y_itor;
		 }
		 else 
			 layer.offset_layer_below_y_=0;
	 }
 }
 
 */
