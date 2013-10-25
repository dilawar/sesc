#include "sesctherm3Dinclude.h"

using namespace std;

/* functional unit placement definition	*/
//FLP_UNIT
ucool_flp_unit::ucool_flp_unit() {
	name_="";
	width_=0;
	height_=0;
	leftx_=0;
	bottomy_=0;
	power_per_unit_area_=0;
	area_=0;
}

ucool_flp_unit::ucool_flp_unit(string name, double width, double height, double leftx, double bottomy) {
    name_=name;
    height_=height;
    width_=width;
    leftx_=leftx;
    bottomy_=bottomy;
	rightx_=leftx_+width_;
	topy_=bottomy_+height_;
    area_=width*height;
    power_per_unit_area_=0;
}

//NOTE: power!=power_per_unit_area
void ucool_flp_unit::set_power(double power, double thickness) {
    power_per_unit_area_=power/(width_*height_*thickness);
}

/* This encapsulates the entire ucool floorplan, this includes all three layers including the inner virtual layer */
//FLOORPLAN
ucool_floorplan::ucool_floorplan(sesctherm3Ddatalibrary* datalibrary){
	datalibrary_ = datalibrary;
}

ostream& operator<<(ostream& os, ucool_floorplan& tmp_floorplan) {
	
	
    os << setprecision(11);
    os << "************ UCOOL FLOORPLAN_DATA ************" << endl;
    os << "Floorplan Defined?:" << ((tmp_floorplan.floorplan_defined_==true)? "yes!" : "no") << endl;
    os << "ucool_flp_units_:[itor][Name        ] [Width       ] [Height      ] [Leftx       ] [Bottomy     ] [Area            ]" << endl;
    for (uint32_t i=0;i<tmp_floorplan.flp_units_.size();i++) {
        os << "ucool_flp_units_:[" << i << "]\t [" ;
        os << setw(12) << tmp_floorplan.flp_units_[i].name_ ;
        os << "] [" << setw(12) << tmp_floorplan.flp_units_[i].width_ ;
        os << "] [" << setw(12) <<  tmp_floorplan.flp_units_[i].height_;
        os << "] [" << setw(12) <<  tmp_floorplan.flp_units_[i].leftx_;
        os << "] [" << setw(12) <<  tmp_floorplan.flp_units_[i].bottomy_ ;
        os << "] [" << setw(16) <<  tmp_floorplan.flp_units_[i].area_ << "]" << endl;
    }
	
    for (uint32_t i=0;i<tmp_floorplan.leftx_coords_.size();i++)
        os << "leftx_coords:[" << i << "] =" << tmp_floorplan.leftx_coords_[i]  <<endl;
	
    for (uint32_t i=0;i<tmp_floorplan.bottomy_coords_.size();i++)
        os << "bottomy_coords_:[" << i << "] =" << tmp_floorplan.bottomy_coords_[i]  <<endl;
	
    os << "max_x_=" << tmp_floorplan.max_x_ << endl;
    os << "max_y_=" << tmp_floorplan.max_y_ << endl;
    os << "ucool_area_=" << tmp_floorplan.ucool_area_ << endl;
    os << "floorplan_name_=" << tmp_floorplan.floorplan_name_ << endl;
	
    return(os);
}

/*
void ucool_floorplan::generate_floorplan_regular(double width, double height){
	int n_coolers=(int)((datalibrary_->config_data_->gen_flp_ucool_pwr_percentage_*datalibrary_->config_data_->max_total_power_)/(datalibrary_->config_data_->gen_flp_pwr_per_cooler_));
	//if we don't want to calculate the total number of coolers based upon the percentage power to use, just get the number
	if(!datalibrary_->config_data_->gen_flp_calc_num_coolers_)
		n_coolers=datalibrary_->config_data_->n_coolers_;
	//calculate the number of rows and columns based upon the floorplan size and the number of ucoolers		 
	int n_rows=(int)(sqrt(n_coolers));
	int n_cols=(int)(n_coolers/n_rows);
	//make sure that the grid is as big as ucoolers. ie. we to make sure that the ucooler fits within the grid size
	//note that this may decrease the number of ucoolers to be placed, however this ensures that they will all fit
	if(width/n_cols < datalibrary_->config_data_->ucool_width_)
		n_cols=(int)(width/datalibrary_->config_data_->ucool_width_);
	if(height/n_rows < datalibrary_->config_data_->ucool_height_)
		n_rows=(int)(height/datalibrary_->config_data_->ucool_height_);				
	
	double x=0;
	double y=0;
	int	ucooler_count = 0;
	for(int i=0;i<n_rows;i++)
		for(int j=0;j<n_cols;j++){
			x=j*(width/n_cols)+ width/(n_cols*2) - datalibrary_->config_data_->ucool_width_/2;
			y=i*(height/n_rows)+ height/(n_rows*2) - datalibrary_->config_data_->ucool_height_/2;
			ucool_flp_unit newunit;
			newunit.name_= "ucooler" +  sesctherm_utilities::stringify(ucooler_count);
			newunit.leftx_=x;
			newunit.bottomy_=y;
			newunit.width_=datalibrary_->config_data_->ucool_width_;
			newunit.height_=datalibrary_->config_data_->ucool_height_;
			newunit.area_=datalibrary_->config_data_->ucool_width_*datalibrary_->config_data_->ucool_height_;
			newunit.rightx_=newunit.leftx_+newunit.width_;
			newunit.topy_=newunit.bottomy_+newunit.height_;
			flp_units_.push_back(newunit);
			leftx_coords_.push_back(x);
			bottomy_coords_.push_back(y);
		}
			//For each of the ucooler units, add the rightx, and topy data points
			
			for (uint32_t i=0;i<flp_units_.size();i++) {
				leftx_coords_.push_back(flp_units_[i].leftx_+flp_units_[i].width_);
				bottomy_coords_.push_back(flp_units_[i].bottomy_+flp_units_[i].height_);
			}
			
			//Sort the FLP units by leftx
			std::sort(flp_units_.begin(),flp_units_.end(),ucool_flp_unit::cmpLeftx());
	
	
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    vector<double>::iterator new_end= std::unique(leftx_coords_.begin(),leftx_coords_.end());
    // delete all elements past new_end 
    leftx_coords_.erase(new_end, leftx_coords_.end());
	
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    new_end=std::unique(bottomy_coords_.begin(),bottomy_coords_.end());
    // delete all elements past new_end 
    bottomy_coords_.erase(new_end, bottomy_coords_.end());
	
    floorplan_defined_=true;
    floorplan_name_="ucooler floorplan";
	
}

void ucool_floorplan::generate_floorplan_pwrdensity(double width, double height){
	
}

void ucool_floorplan::generate_floorplan_temperature(double width, double height){
	
	
}
void ucool_floorplan::generate_floorplan_random(double width, double height){
	
}

void ucool_floorplan::generate_floorplan(double width, double height){
	
	switch(datalibrary_->config_data_->ucool_generation_policy_)
	{
		case (UCOOL_PLACEMENT_REGULAR):
			generate_floorplan_regular(width,height);
			break;
			
		case(UCOOL_PLACEMENT_PWRDENSITY):
			generate_floorplan_pwrdensity(width,height);
			break;
			
		case(UCOOL_PLACEMENT_TEMPERATURE):
			generate_floorplan_temperature(width,height);
			break;
			
		case(UCOOL_PLACEMENT_RANDOM):
			generate_floorplan_random(width,height);
			break;
			
		default:
			cerr << "FATAL: invalid ucool placement policy" << endl;
			exit(1);
	}
}

*/

void ucool_floorplan::get_floorplan_flpfile(ifstream& if_infile){
	std::string line;
    std::vector<string> tokens;
	
    floorplan_defined_=false;
    int32_t line_number=0;
    max_x_=0;
    max_y_=0;
    ucool_area_=0;
    double ucooler_count=0;
    if (if_infile == NULL)
        sesctherm_utilities::fatal("Unable to read Floorplan File");
	
    while (getline(if_infile, line)) {
        line_number++;
        if (line.empty())
            continue; //ignore blank lines
        sesctherm_utilities::Tokenize(line, tokens, " \t"); //tokensize the line by space and tabs
        if (tokens[0] == "#")
            continue; //skip comments
        if (tokens.size()!=2) {
            cerr << "Error Reading Ucool FLP File: Invalid line-length [Line" << line_number << endl;
            exit(1);
        }
        ucool_flp_unit newunit;
        newunit.name_= "ucooler" +  sesctherm_utilities::stringify(ucooler_count);
        newunit.leftx_=sesctherm_utilities::convertToDouble(tokens[0]);
        newunit.bottomy_=sesctherm_utilities::convertToDouble(tokens[1]);
        newunit.width_=datalibrary_->config_data_->ucool_width_;
        newunit.height_=datalibrary_->config_data_->ucool_height_;
        newunit.area_=datalibrary_->config_data_->ucool_width_*datalibrary_->config_data_->ucool_height_;
		newunit.rightx_=newunit.leftx_+newunit.width_;
		newunit.topy_=newunit.bottomy_+newunit.height_;
        flp_units_.push_back(newunit);
        leftx_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[0]));
        bottomy_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[1]));
    }
	
    //For each of the ucooler units, add the rightx, and topy data points
	
    for (uint32_t i=0;i<flp_units_.size();i++) {
        leftx_coords_.push_back(flp_units_[i].leftx_+flp_units_[i].width_);
        bottomy_coords_.push_back(flp_units_[i].bottomy_+flp_units_[i].height_);
    }
	
    //Sort the FLP units by leftx
    std::sort(flp_units_.begin(),flp_units_.end(),ucool_flp_unit::cmpLeftx());
	
	
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    vector<double>::iterator new_end= std::unique(leftx_coords_.begin(),leftx_coords_.end());
    // delete all elements past new_end 
    leftx_coords_.erase(new_end, leftx_coords_.end());
	
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    new_end=std::unique(bottomy_coords_.begin(),bottomy_coords_.end());
    // delete all elements past new_end 
    bottomy_coords_.erase(new_end, bottomy_coords_.end());
	
    floorplan_defined_=true;
    floorplan_name_="ucooler floorplan";
}

void ucool_floorplan::get_floorplan_sescconf(){
	// READ the floorplan parameters
    const char *floorplanSec = SescConf->getCharPtr("","floorplan");
    int32_t min = SescConf->getRecordMin(floorplanSec,"blockDescr") ;
    int32_t max = SescConf->getRecordMax(floorplanSec,"blockDescr") ;
	std::string line;
    std::vector<string> tokens;
	
    floorplan_defined_=false;
    int32_t line_number=0;
    max_x_=0;
    max_y_=0;
    ucool_area_=0;
    double ucooler_count=0;
    for (int id=min;id<=max;id++) {
        const char *name = SescConf->getCharPtr(floorplanSec,"blockDescr",id);
		line = name;
        line_number++;
        if (line.empty())
            continue; //ignore blank lines
        sesctherm_utilities::Tokenize(line, tokens, " \t"); //tokensize the line by space and tabs
        if (tokens.size()!=2) {
            cerr << "Error Reading Ucool within Sescconf: Invalid line-length [Line" << line_number << endl;
            exit(1);
        }
        ucool_flp_unit newunit;
        newunit.name_= "ucooler" +  sesctherm_utilities::stringify(ucooler_count);
        newunit.leftx_=sesctherm_utilities::convertToDouble(tokens[0]);
        newunit.bottomy_=sesctherm_utilities::convertToDouble(tokens[1]);
        newunit.width_=datalibrary_->config_data_->ucool_width_;
        newunit.height_=datalibrary_->config_data_->ucool_height_;
        newunit.area_=datalibrary_->config_data_->ucool_width_*datalibrary_->config_data_->ucool_height_;
		newunit.rightx_=newunit.leftx_+newunit.width_;
		newunit.topy_=newunit.bottomy_+newunit.height_;
        flp_units_.push_back(newunit);
        leftx_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[0]));
        bottomy_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[1]));
    }
	
    //For each of the ucooler units, add the rightx, and topy data points
	
    for (uint32_t i=0;i<flp_units_.size();i++) {
        leftx_coords_.push_back(flp_units_[i].leftx_+flp_units_[i].width_);
        bottomy_coords_.push_back(flp_units_[i].bottomy_+flp_units_[i].height_);
    }
	
    //Sort the FLP units by leftx
    std::sort(flp_units_.begin(),flp_units_.end(),ucool_flp_unit::cmpLeftx());
	
	
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    vector<double>::iterator new_end= std::unique(leftx_coords_.begin(),leftx_coords_.end());
    // delete all elements past new_end 
    leftx_coords_.erase(new_end, leftx_coords_.end());
	
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    new_end=std::unique(bottomy_coords_.begin(),bottomy_coords_.end());
    // delete all elements past new_end 
    bottomy_coords_.erase(new_end, bottomy_coords_.end());
	
    floorplan_defined_=true;
    floorplan_name_="ucooler floorplan";
	
}

//generate is false by default
void ucool_floorplan::get_floorplan(ifstream& if_infile, double width, double height, bool get_from_sescconf, bool generate) {
	if(generate)
		I(0);
		//generate_floorplan(width,height);
	else if(get_from_sescconf)
		get_floorplan_sescconf();
	else
		get_floorplan_flpfile(if_infile);
}


// This offsets the data values
void ucool_floorplan::offset_floorplan(double x_amount, double y_amount){
    for (uint32_t i=0;i<leftx_coords_.size();i++) {
        leftx_coords_[i]+=x_amount;
    }
    for (uint32_t i=0;i<bottomy_coords_.size();i++) {
        bottomy_coords_[i]+=y_amount;
    }
    for (uint32_t i=0;i<flp_units_.size();i++) {
        flp_units_[i].bottomy_+=y_amount;
        flp_units_[i].leftx_+=x_amount;
    }
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    //store the maximum leftx datapoint (ucool width)
    max_x_=leftx_coords_[leftx_coords_.size()-1];
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    //store the maximum bottomy datapoint (ucool height)
    max_y_=bottomy_coords_[bottomy_coords_.size()-1];
    //now calculate the ucool area (will be the same)
    double tmp_ucool_area=(max_x_-leftx_coords_[0])*(max_y_-bottomy_coords_[0]);
    if (!EQ(tmp_ucool_area,ucool_area_) && ucool_area_!=0) { //if the new ucool area is different (and we are not using the ucooler floorplan)
        cerr << "FATAL: offset_floorplan: new ucool area (after offset) different (should be the same ALWAYS)" << endl;
        cerr << "new ucool area:[" << tmp_ucool_area << "] old ucool area:[" << ucool_area_ << "]" << endl;
        cerr << "error:" << ucool_area_-tmp_ucool_area << endl;
        exit(1);
    }
}

ucool_flp_unit & ucool_floorplan::find_flp_unit(double x, double y){
    for (uint32_t i=0;i<flp_units_.size();i++) {
        if (LT(flp_units_[i].bottomy_,y) &&
            GT(flp_units_[i].bottomy_ + flp_units_[i].height_,  y) &&
            LT(flp_units_[i].leftx_ , x) &&
            GT(flp_units_[i].leftx_ + flp_units_[i].width_ , x))
            return(flp_units_[i]);
    }
    cerr << "find_flp_unit: [" << x << "][" << y << "] Does not exist. Use flp_unit_exists to check first" << endl;
    exit(1);
}

bool ucool_floorplan::flp_unit_exists(double x, double y){
	
    for (uint32_t i=0;i<flp_units_.size();i++) {
        if (LT(flp_units_[i].bottomy_,y) &&
            GT(flp_units_[i].bottomy_ + flp_units_[i].height_,  y) &&
            LT(flp_units_[i].leftx_ , x) &&
            GT(flp_units_[i].leftx_ + flp_units_[i].width_ , x)) {
#ifdef _SESCTHERM_DEBUG
        //    cerr << "Found FLP unit at:[" << x << "][" << y << "]" << endl;
#endif
            return(true);
        }
    }
    return(false);
}
/*


// total ucool width	
double ucool_floorplan::get_total_width(void){
    return(max_x_);
}

// total ucool height 
double ucool_floorplan::get_total_height(void){
    return(max_y_);
}

double ucool_floorplan::get_total_area(void){
    return(ucool_area_);
}

vector<ucool_flp_unit> & ucool_floorplan::get_flp_units(void){
    return(flp_units_);
}

vector<ucool_flp_unit> & ucool_floorplan::get_leftx_coords(void){
    return(flp_units_);
}

vector<ucool_flp_unit> & ucool_floorplan::get_bottomy_coords(void){
    return(flp_units_);
}

bool ucool_floorplan::is_floorplan_defined(){
    return(floorplan_defined_);
}

string ucool_floorplan::get_floorplan_name(){
    return(floorplan_name_);
}






// Analog Temperature Averaging for Preemptive Ucool On-Off policy
//  Instead of implementing this in digital logic that would require comparison operations
//  that would substantially implicate the required hardware and would require ADC,
//  this could be implemented in hardware like the following (simple voltage comparator):
//  This circuit effectively performs a MAX operation selecting the larget
//  of the two signals on the output
//   |--------------
//   |       |     |_
//   |       ---- -|  \
//  	| 		 opamp|   > --- resistor ----
//   V1   |------ +|_ /            		|
//   |	 V2			|					|
//   |    |  		|					|
//   |------------------------------------
//   |
//  GND

//this policy uses power-saving features to only turn on ucoolers when needed
void ucool_floorplan::ucool_weighted_region(){
	double ave_local_temp;
	double ave_regional_temp;
	vector<model_unit> located_units;
 	for(uint32_t i=0;i<flp_units_.size();i++){
	 	//calculate the local temperature
		ave_local_temp=0;
		for(uint32_t j=0;j<flp_units_[i].model_units_.size();i++)
	 		ave_local_temp+=flp_units_[i].model_units_[j]->get_temperature();
	 	ave_local_temp=ave_local_temp/flp_units_[i].model_units_.size();
		//find a list of all the model units that exist within the regional area around the ucooler
	 	sesctherm3Dmodel::locate_model_units(1,
											 located_units,
											 sesctherm_utilities::max(flp_units_[i].leftx_ - datalibrary_->config_data_->ucool_weighted_region_size_,0),
											 sesctherm_utilities::max(flp_units_[i].bottomy_ - datalibrary_->config_data_->ucool_weighted_region_size_,0),
											 sesctherm_utilities::min(flp_units_[i].rightx_ + datalibrary_->config_data_->ucool_weighted_region_size_,datalibrary_->chip_floorplan_->width_),
											 sesctherm_utilities::min(flp_units_[i].topy_ + datalibrary_->config_data_->ucool_weighted_region_size_,datalibrary_->chip_floorplan_->height_),
											 datalibrary_);
		//calculate the regional temperature
		ave_regional_temp=0;
 		for(uint32_t j=0;j<located_units.size();j++)
			ave_regional_temp+=located_units[j].get_temperature();
 		ave_regional_temp=ave_regional_temp/located_units.size();
 		//if the local region is less than the maximum on-temperature 
		//(when ucooler should be turned off)
 		if(ave_local_temp<=datalibrary_->config_data_->ucool_weighted_region_max_temp_){
 			//if the local temp isn't too hot to turn on ucooler in anticipation of a hot regional temperature
 			if(ave_regional_temp>datalibrary_->config_data_->ucool_weighted_region_min_temp_)
 				flp_units_[i].on_off_=true;	//turn on ucooler
 			else if(ave_local_temp<datalibrary_->config_data_->ucool_weighted_region_min_temp_)
 				flp_units_[i].on_off_=false; //turn off ucooler as the local and regional temps are cool
 		}
 		//otherwise the local ucooler is too hot (turn/keep it off)
 		else
 			flp_units_[i].on_off_=false;	//turn/keep it off
 	}	
 	
}


void ucool_floorplan::perform_ucool_dtm(){
	
	
	
	
}

//turn all ucoolers on or off depending upon the average chip temperature 	
bool ucool_floorplan::ucool_on_off(double min_temp, double max_temp){
 	double chip_temp_ave=0;
	//calculate the average chip temperature
 	  for (uint32_t y_itor=0;y_itor<datalibrary_->chip_floorplan_dyn_->nrows();y_itor++)
		  for (uint32_t x_itor=0;x_itor<datalibrary_->chip_floorplan_dyn_->ncols();x_itor++)          
			  chip_temp_ave+=(*datalibrary_->chip_floorplan_dyn_)[x_itor][y_itor].get_temperature();
	  chip_temp_ave=chip_temp_ave/(datalibrary_->chip_floorplan_dyn_->nrows()*datalibrary_->chip_floorplan_dyn_->ncols());
	  
	  if(chip_temp_ave>max_temp)
		  return(false);	
	else if(chip_temp_ave>min_temp && chip_temp_ave <max_temp)
		return(true);
	else
		return(false);
}

//if the average temperature is any given region is too high, turn off all ucoolers and perform
//voltage scaling, frequency scaling or otherwise
bool ucool_floorplan::ucool_on_off_regional(int nregions, double min_temp, double max_temp){
 	 	double region_temp_ave=0;
	vector<model_unit> located_units;
 	 	int rows=(int)sqrt(nregions);
 	 	int cols=(int)(nregions/rows);
		for(uint32_t i=0;i<datalibrary_->chip_floorplan_dyn_->nrows();i++){
			for(uint32_t j=0;j<datalibrary_->chip_floorplan_dyn_->ncols();j++){ 	 				
				sesctherm3Dmodel::locate_model_units(1,
													 located_units,
													 j*datalibrary_->chip_floorplan_->width_/datalibrary_->chip_floorplan_dyn_->ncols(),
													 i*datalibrary_->chip_floorplan_->height_/datalibrary_->chip_floorplan_dyn_->nrows(),
													 j*datalibrary_->chip_floorplan_->width_/datalibrary_->chip_floorplan_dyn_->ncols()+datalibrary_->config_data_->ucool_on_off_region_size_,
													 i*datalibrary_->chip_floorplan_->height_/datalibrary_->chip_floorplan_dyn_->nrows()+datalibrary_->config_data_->ucool_on_off_region_size_,
													 datalibrary_);
				region_temp_ave=0;
				for(uint32_t k=0;k<located_units.size();k++){
					region_temp_ave+=located_units[k].get_temperature();
				}
				region_temp_ave=region_temp_ave/located_units.size();
				if(region_temp_ave>max_temp)
					return(false);	
				else if(region_temp_ave > min_temp && region_temp_ave < max_temp)
					return(true);
				else
					return(false);
			} 	
		}	
		return(false);
}

void ucool_floorplan::ucool_always_on(){
 	for(uint32_t i=0;i<flp_units_.size();i++){
		flp_units_[i].on_off_=true;	//turn all ucoolers on
	}
}
void ucool_floorplan::ucool_always_off(){
	for(uint32_t i=0;i<flp_units_.size();i++){
		flp_units_[i].on_off_=true;	//turn all ucoolers off
	}
}

void ucool_floorplan::ucool_PID(){
	
	
	
	
}

*/
