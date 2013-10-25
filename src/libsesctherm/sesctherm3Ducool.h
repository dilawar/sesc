/* Placement Strategies:
* MESH_STATIC:
* 				Start at the center of the grid, place N micro-coolers per grid unit, running outwards in circular patterm
* 				until all of the ucooler units are exhausted
* MESH_POWER_DEPENDENT:
* 				Create a mesh, storing the power densities of each grid unit. Start with the hottest regions. Place N microcoolers
* 				per W/m^2 (heat density) until grid unit is full. Note: grid unit width and height must be greater than or equal to 
* 				the micro-cooler 
* MESH_TEMPERATURE_DEPENDENT:
* 				Create a mesh, store the max temperature of each grid unit (no micro-coolers used).
* 			
* 
* 
* 
*/

#ifndef SESCTHERM3D_UCOOL_H
#define SESCTHERM3D_UCOOL_H

enum { UCOOL_PLACEMENT_REGULAR, UCOOL_PLACEMENT_PWRDENSITY, UCOOL_PLACEMENT_TEMPERATURE, UCOOL_PLACEMENT_RANDOM };


class ucool_flp_unit;
class ucool_floorplan;


/* functional unit placement definition	*/
class ucool_flp_unit {
public:
    ucool_flp_unit();
    ucool_flp_unit(string name, double width, double height, double leftx, double bottomy);
    friend ostream& operator<<(ostream& os, ucool_flp_unit& tmp_ucool_flp_unit);
	struct cmpWidth {
		bool operator()(const ucool_flp_unit& a, const ucool_flp_unit& b)
	{
            return(LT(a.width_,b.width_));
	}
	};
	struct cmpHeight {
		bool operator()(const ucool_flp_unit& a, const ucool_flp_unit& b)
	{
            return(LT(a.height_, b.height_));
	}
	};
	
	struct cmpLeftx {
		bool operator()(const ucool_flp_unit& a, const ucool_flp_unit& b)
	{
            return(LT(a.leftx_, b.leftx_));
	}
	};
	
	struct cmpBottomy {
		bool operator()(const ucool_flp_unit& a, const ucool_flp_unit& b)
	{
			return(LT(a.bottomy_, b.bottomy_));
	}
	};
	
	struct cmpArea {
		bool operator()(const ucool_flp_unit& a, const ucool_flp_unit& b)
	{
			return(LT(a.area_, b.area_));
	}
    };
	
    //NOTE: power!=power_per_unit_area
    void set_power(double power, double thickness);
    string name_;
    double width_;
    double height_;
    double leftx_;
    double bottomy_;
	double rightx_;
	double topy_;
    double area_;
    vector<double>     model_unit_percentages_; //this is a list of the percentages of each of the dependent model units
    vector<model_unit*> model_units_;			//this is a list of the model units that correspond to this ucool flp unit
    double power_per_unit_area_; //this is the current power dissipated by the flp (W/m^3)
	bool on_off_;
	double hot_spot_count_;
	double hot_spot_duration_;
	
};

/* This encapsulates the entire ucool floorplan, this includes all three layers including the inner virtual layer */
class ucool_floorplan {
public:
    ucool_floorplan(sesctherm3Ddatalibrary* datalibrary);
    friend ostream& operator<<(ostream& os, ucool_floorplan& tmp_floorplan);
/*	
	void generate_floorplan(double width, double height);
	
	void generate_floorplan_regular(double width, double height);
	
	void generate_floorplan_pwrdensity(double width, double height);
	
	void generate_floorplan_temperature(double width, double height);
	
	void generate_floorplan_random(double width, double height);
*/	
	
	void get_floorplan_flpfile(ifstream& if_infile);
	
	void get_floorplan_sescconf();
	
    // read the .flp file, store information to ucool_flp_units
    // store information to leftx_coords (sorted list of x-values)
    // store information to bottom_corrds (corted list of y-values)
	void get_floorplan(ifstream& if_infile, double width, double height, bool get_from_sescconf, bool generate = false);  
	
    // This offsets the data values
    void offset_floorplan(double x_amount, double y_amount);
  
  	bool flp_unit_exists(double x, double y);
  ucool_flp_unit & find_flp_unit(double x, double y);
  
  
	/*  
	//These at the DTM policies
	
	void ucool_weighted_region(); 	
	
	void perform_ucool_dtm();
	
	//turn all ucoolers on or off depending upon the average chip temperature 	
	bool ucool_on_off(double min_temp, double max_temp);
	
	//if the average temperature is any given region is too high, turn off all ucoolers and perform
	//voltage scaling, frequency scaling or otherwise
	bool ucool_on_off_regional(int nregions, double min_temp, double max_temp);
	
	void ucool_always_on();
	
	void ucool_always_off();
	
	
	void ucool_PID();
	
	
	// total ucool width	
	double get_total_width(void);
	
	// total ucool height 
	double get_total_height(void);
	double get_total_area(void);
	
	
	
	vector<ucool_flp_unit> & get_flp_units(void);
	
	vector<ucool_flp_unit> & get_leftx_coords(void);
	
	vector<ucool_flp_unit> & get_bottomy_coords(void);
	
	bool is_floorplan_defined();
	
	string get_floorplan_name();
	
	

	*/
	vector<ucool_flp_unit>     flp_units_;
	vector<double>     leftx_coords_;
	vector<double>     bottomy_coords_;
	double max_x_;
	double max_y_;
	double ucool_area_;
	double ucool_width_;
	double ucool_height_;
	string floorplan_name_;
	bool floorplan_defined_;
	sesctherm3Ddatalibrary* datalibrary_;
};



#endif
