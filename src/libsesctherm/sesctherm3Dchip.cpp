#include "sesctherm3Dinclude.h"



/* functional unit placement definition	*/
//FLP_UNIT
chip_flp_unit::chip_flp_unit(sesctherm3Ddatalibrary* datalibrary) {
	datalibrary_=datalibrary;

	for(uint32_t i=0;i<datalibrary_->all_layers_info_.size();i++){
		std::vector<model_unit*> located_units_layer;
		located_units_layer.clear();
		located_units_.push_back(located_units_layer);
	}
	name_="";
	width_=0;
	height_=0;
	leftx_=0;
	bottomy_=0;
	power_per_unit_area_=0;
	area_=0;
    dynamic_power_=0;
	leakage_power_=0;
    hot_spot_count_=0;
    hot_spot_duration_=0;
}

chip_flp_unit::chip_flp_unit(sesctherm3Ddatalibrary* datalibrary, int32_t id, string& name, double width, double height, double leftx, double bottomy, double chipdensity, double interconnectdensity) {
	datalibrary_=datalibrary;
	id_=id;
    name_=name;
    height_=height;
    width_=width;
    leftx_=leftx;
    bottomy_=bottomy;
    area_=width*height;
	chip_density_=chipdensity;
	interconnect_density_=interconnectdensity;
    power_per_unit_area_=0;
    dynamic_power_=0;
	leakage_power_=0;
    hot_spot_count_=0;
    hot_spot_duration_=0;
	located_units_.clear();
	for(uint32_t i=0;i<20;i++){
		std::vector<model_unit*> located_units_layer;
		located_units_layer.clear();
		located_units_.push_back(located_units_layer);
	}
}

//NOTE: power!=power_per_unit_area
void chip_flp_unit::set_power(double power, double thickness) {
	power_=power;
    power_per_unit_area_=power/(width_*height_*thickness);
}

//NOTE: power!=power_per_unit_area
void chip_flp_unit::set_max_power(double max_dynamic_power) {
	max_dynamic_power_=max_dynamic_power;
}

/* This encapsulates the entire chip chip_floorplan, this includes all three layers including the inner virtual layer */
//FLOORPLAN
chip_floorplan::chip_floorplan(sesctherm3Ddatalibrary* datalibrary){
	datalibrary_=datalibrary;
}

ostream& operator<<(ostream& os, chip_floorplan& tmp_floorplan) {
	
	
    os << setprecision(11);
    os << "************ chip_floorplan_DATA ************" << endl;
    os << "chip_floorplan Defined?:" << ((tmp_floorplan.floorplan_defined_==true)? "yes!" : "no") << endl;
    os << "flp_units_:[itor][Name        ] [Width       ] [Height      ] [Leftx       ] [Bottomy     ] [Area            ]" << endl;
    for (uint32_t i=0;i<tmp_floorplan.flp_units_.size();i++) {
        os << "flp_units_:[" << i << "]\t [" ;
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
    os << "chip_area_=" << tmp_floorplan.area_ << endl;
    os << "chip_floorplan_name_=" << tmp_floorplan.floorplan_name_ << endl;
	
    return(os);
}

//FIXME: this needs to handle densities (not currently implemented)
void chip_floorplan::get_floorplan_flpfile(){
	I(0);
   
	/*
	  std::string line;
    std::vector<string> tokens;
    floorplan_defined_=false;
    int32_t line_number=0;
	ifstream& if_infile=datalibrary_->if_flpfile_;
    if (if_infile == NULL)
        sesctherm_utilities::fatal("Unable to read chip_floorplan File");
	int id=0;
    while (getline(if_infile,line)) {
        line_number++;
        if (line.empty())
            continue; //ignore blank lines
        sesctherm_utilities::Tokenize(line, tokens, " \t"); //tokensize the line by space and tabs
        if (tokens[0] == "#")
            continue;//skip comments
			if (tokens.size()!=5) {
				cerr << "Error Reading Chip FLP File: Invalid line-length [Line" << line_number << "]" << endl;
				cerr << "Number of tokens found" << tokens.size() << endl;
				exit(1);
			}
			chip_flp_unit newunit(id,
								  tokens[0],
								  sesctherm_utilities::convertToDouble(tokens[1]),
								  sesctherm_utilities::convertToDouble(tokens[2]),
								  sesctherm_utilities::convertToDouble(tokens[3]),
								  sesctherm_utilities::convertToDouble(tokens[4]));
			flp_units_.push_back(newunit);
			leftx_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[3]));
			bottomy_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[4]));
			id++;
    }
	
	vector<chip_flp_unit> tmp_flp_units=flp_units_; 
    //Sort the FLP units by leftx
    std::sort(tmp_flp_units.begin(),tmp_flp_units.end(),chip_flp_unit::cmpLeftx());
	
    chip_flp_unit* tmpunit=NULL;
    //Add data point for right-most x-point
    tmpunit=&(tmp_flp_units[tmp_flp_units.size()-1]);
    leftx_coords_.push_back(tmpunit->leftx_+tmpunit->width_);
	
    //sort the FLP units by bottomy
    std::sort(tmp_flp_units.begin(),tmp_flp_units.end(),chip_flp_unit::cmpBottomy());
	
    //Add data point for top-most y-point
    tmpunit=&(tmp_flp_units[tmp_flp_units.size()-1]);
    bottomy_coords_.push_back(tmpunit->bottomy_+tmpunit->height_);
	
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    vector<double>::iterator new_end= std::unique(leftx_coords_.begin(),leftx_coords_.end());
    // delete all elements past new_end
    leftx_coords_.erase(new_end, leftx_coords_.end());
	
    //store the maximum leftx datapoint (chip width)
    max_x_=leftx_coords_[leftx_coords_.size()-1];
	
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    new_end=std::unique(bottomy_coords_.begin(),bottomy_coords_.end());
    // delete all elements past new_end
    bottomy_coords_.erase(new_end, bottomy_coords_.end());
	
    //store the maximum bottomy datapoint (chip height)
    max_y_=bottomy_coords_[bottomy_coords_.size()-1];
	
    //now calculate the chip area
    area_=max_x_*max_y_;
    //successfully read the chip_floorplan
    floorplan_defined_=true;
    floorplan_name_="chip floorplan";
    width_=max_x_;
    height_=max_y_;
	*/
	
}

void chip_floorplan::get_floorplan_sescconf(int flp_num){

	std::vector<string> tokens;
	std::string line;
	int min,max;
	//Determine which floorplan we are reading from
    const char *thermSec = SescConf->getCharPtr("","thermal");
	const char *floorplanSec = SescConf->getCharPtr(thermSec,"floorplan",flp_num);	
	// READ the floorplan parameters
    min = SescConf->getRecordMin(floorplanSec,"blockDescr") ;
    max = SescConf->getRecordMax(floorplanSec,"blockDescr") ;
	
    for (int id=min;id<=max;id++) {
        const char *blockdescr = SescConf->getCharPtr(floorplanSec,"blockDescr",id);		
		line=blockdescr;
		sesctherm_utilities::Tokenize(line, tokens, " "); //tokensize the line by spaces
		if (tokens.size()!=5) {
            cerr << "Error Reading Chip FLP Description in Sescconf!" << endl;
            exit(1);
        }
		line=tokens[0];
		double chip_density=SescConf->getDouble(floorplanSec, "blockchipDensity", id);
		double interconnect_density=SescConf->getDouble(floorplanSec, "blockinterconnectDensity", id);
        chip_flp_unit newunit(&(*datalibrary_),
							  id-min,
							  line,
							  sesctherm_utilities::convertToDouble(tokens[1]),
							  sesctherm_utilities::convertToDouble(tokens[2]),
							  sesctherm_utilities::convertToDouble(tokens[3]),
							  sesctherm_utilities::convertToDouble(tokens[4]),
							  chip_density,
							  interconnect_density
							  );
        flp_units_.push_back(newunit);
		double leftx_=sesctherm_utilities::convertToDouble(tokens[3]);
		double bottomy_=sesctherm_utilities::convertToDouble(tokens[4]);

        leftx_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[3]));
        bottomy_coords_.push_back(sesctherm_utilities::convertToDouble(tokens[4]));		
    }  
	vector<chip_flp_unit> tmp_flp_units=flp_units_; 
	//Sort the FLP units by leftx
    std::sort(tmp_flp_units.begin(),tmp_flp_units.end(),chip_flp_unit::cmpLeftx());
	
    chip_flp_unit* tmpunit=NULL;
    //Add data point for right-most x-point
    tmpunit=&(tmp_flp_units[tmp_flp_units.size()-1]);
    leftx_coords_.push_back(tmpunit->leftx_+tmpunit->width_);
	
    //sort the FLP units by bottomy
    std::sort(tmp_flp_units.begin(),tmp_flp_units.end(),chip_flp_unit::cmpBottomy());
	
    //Add data point for top-most y-point
    tmpunit=&(tmp_flp_units[tmp_flp_units.size()-1]);
    bottomy_coords_.push_back(tmpunit->bottomy_+tmpunit->height_);
	
    //sort the leftx vector by leftx datapoints
    std::sort(leftx_coords_.begin(),leftx_coords_.end());
    vector<double>::iterator new_end= std::unique(leftx_coords_.begin(),leftx_coords_.end());
    // delete all elements past new_end
    leftx_coords_.erase(new_end, leftx_coords_.end());
	
    //store the maximum leftx datapoint (chip width)
    max_x_=leftx_coords_[leftx_coords_.size()-1];
	
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    new_end=std::unique(bottomy_coords_.begin(),bottomy_coords_.end());
    // delete all elements past new_end
    bottomy_coords_.erase(new_end, bottomy_coords_.end());
	
    //store the maximum bottomy datapoint (chip height)
    max_y_=bottomy_coords_[bottomy_coords_.size()-1];
	
    //now calculate the chip area
    area_=max_x_*max_y_;
    //successfully read the chip_floorplan
    floorplan_defined_=true;
    floorplan_name_="chip floorplan";
    width_=max_x_;
    height_=max_y_;
	
	
	double chip_width = datalibrary_->config_data_->chip_width_;
	double chip_height = datalibrary_->config_data_->chip_height_;
	
	//Scale the leftx/topy datapoints to fit the size of the chip
			
			double rangex=leftx_coords_[leftx_coords_.size()-1] - leftx_coords_[0];
			double rangey=bottomy_coords_[bottomy_coords_.size()-1] - bottomy_coords_[0];
			
				//scale each of the leftx/bottomy coordinates to match the actual size of the chip
				//x_coord/rangex*chip_width=new_x_coord
				//y_coord/rangey*chip_height=new_y_coord
			for(int i=0;i<(int)leftx_coords_.size();i++)
				leftx_coords_[i]=((leftx_coords_[i]-leftx_coords_[0])/rangex)*chip_width+leftx_coords_[0];
			
			for(int i=0;i<(int)bottomy_coords_.size();i++)
				bottomy_coords_[i]=((bottomy_coords_[i]-bottomy_coords_[0])/rangey)*chip_height+bottomy_coords_[0];
			
			for(int i=0;i<(int)flp_units_.size();i++){
				flp_units_[i].leftx_=((flp_units_[i].leftx_-leftx_coords_[0])/rangex)*chip_width+leftx_coords_[0];
				flp_units_[i].bottomy_=((flp_units_[i].bottomy_-bottomy_coords_[0])/rangey)*chip_height+bottomy_coords_[0];
				flp_units_[i].width_=(flp_units_[i].width_/rangex)*chip_width;
				flp_units_[i].height_=(flp_units_[i].height_/rangey)*chip_height;
				flp_units_[i].area_=flp_units_[i].width_*flp_units_[i].height_;
			}
			
			max_x_=leftx_coords_[leftx_coords_.size()-1];
			max_y_=bottomy_coords_[bottomy_coords_.size()-1];
			area_=max_x_*max_y_;
			width_=chip_width;
			height_=chip_height;
			

}

// read the .flp file, store information to flp_units
// store information to leftx_coords (sorted list of x-values)
// store information to bottom_corrds (corted list of y-values)
void chip_floorplan::get_floorplan(bool get_from_sescconf, int32_t flp_num) {
	if(get_from_sescconf)
		get_floorplan_sescconf(flp_num);
	else
		get_floorplan_flpfile();
}

// This offsets the data values
void chip_floorplan::offset_floorplan(double x_amount, double y_amount){
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
    //store the maximum leftx datapoint (chip width)
    max_x_=leftx_coords_[leftx_coords_.size()-1];
    //sort the bottom vector by bottomy datapoints
    std::sort(bottomy_coords_.begin(),bottomy_coords_.end());
    //store the maximum bottomy datapoint (chip height)
    max_y_=bottomy_coords_[bottomy_coords_.size()-1];
    //now calculate the chip area (will be the same)
    double tmp_area=(max_x_-leftx_coords_[0])*(max_y_-bottomy_coords_[0]);
    if (!EQ(tmp_area,area_) && area_!=0) { //if the new chip area is different (and we are not using the ucooler floorplan)
        cerr << "FATAL: offset_floorplan: new chip area (after offset) different (should be the same ALWAYS)" << endl;
        cerr << "new chip area:[" << tmp_area << "] old chip area:[" << area_ << "]" << endl;
        cerr << "error:" << area_-tmp_area << endl;
        exit(1);
    }
}
/* total chip width	*/
double chip_floorplan::get_total_width(void){
    return(max_x_);
}

/* total chip height */
double chip_floorplan::get_total_height(void){
    return(max_y_);
}

double chip_floorplan::get_total_area(void){
    return(area_);
}

vector<chip_flp_unit> & chip_floorplan::get_flp_units(void){
    return(flp_units_);
}

vector<chip_flp_unit> & chip_floorplan::get_leftx_coords(void){
    return(flp_units_);
}

vector<chip_flp_unit> & chip_floorplan::get_bottomy_coords(void){
    return(flp_units_);
}

bool chip_floorplan::is_floorplan_defined(){
    return(floorplan_defined_);
}

string chip_floorplan::get_floorplan_name(){
    return(floorplan_name_);
}
bool chip_floorplan::flp_unit_exists(double x, double y){
	
    for (uint32_t i=0;i<flp_units_.size();i++) {
        if (LE(flp_units_[i].bottomy_,y) &&
            GE(flp_units_[i].bottomy_ + flp_units_[i].height_,  y) &&
            LE(flp_units_[i].leftx_ , x) &&
            GE(flp_units_[i].leftx_ + flp_units_[i].width_ , x)) {
#ifdef _SESCTHERM_DEBUG
  //          cerr << "Found FLP unit at:[" << x << "][" << y << "]" << endl;
#endif
            return(true);
        }
    }
    return(false);
}
chip_flp_unit & chip_floorplan::find_flp_unit(double x, double y){
		double bottomy_;
		double leftx_;
		double height_;
		double width_;
    for (uint32_t i=0;i<flp_units_.size();i++) {
		bottomy_=flp_units_[i].bottomy_;
		leftx_=flp_units_[i].leftx_;
		height_=flp_units_[i].height_;
		width_=flp_units_[i].width_;
        if (LE(flp_units_[i].bottomy_,y) &&
            GE(flp_units_[i].bottomy_ + flp_units_[i].height_,  y) &&
            LE(flp_units_[i].leftx_ , x) &&
            GE(flp_units_[i].leftx_ + flp_units_[i].width_ , x))
            return(flp_units_[i]);			
    }
    cerr << "find_flp_unit: [" << x << "][" << y << "] Does not exist. Use flp_unit_exists to check first" << endl;
    exit(1);
}


