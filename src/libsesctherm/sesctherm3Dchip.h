#ifndef SESCTHERM3D_FLP_H
#define SESCTHERM3D_FLP_H
class chip_flp_unit;
class chip_floorplan;


/* functional unit placement definition	*/
class chip_flp_unit {
public:
  chip_flp_unit(){}
  chip_flp_unit(sesctherm3Ddatalibrary* datalibrary);
  chip_flp_unit(sesctherm3Ddatalibrary* datalibrary, int32_t id, string& name, double width, double height, double leftx, double bottomy, double chipdensity, double interconnectdensity);
  friend ostream& operator<<(ostream& os, chip_flp_unit& tmp_chip_flp_unit);

  struct cmpWidth {
    bool operator()(const chip_flp_unit& a, const chip_flp_unit& b) const {
      return(LT(a.width_,b.width_));
    }
  };

  struct cmpHeight {
    bool operator()(const chip_flp_unit& a, const chip_flp_unit& b) const {
      return(LT(a.height_, b.height_));
    }
  };

  struct cmpLeftx {
    bool operator()(const chip_flp_unit& a, const chip_flp_unit& b) const {
      return(LT(a.leftx_, b.leftx_));
    }
  };
  
  struct cmpBottomy {
    bool operator()(const chip_flp_unit& a, const chip_flp_unit& b) const{
      return(LT(a.bottomy_, b.bottomy_));
    }
  };
  
  struct cmpArea {
    bool operator()(const chip_flp_unit& a, const chip_flp_unit& b) const {
      return(LT(a.area_, b.area_));
    }
  };

  //stores the line number/block description number where the declaraction occurs
  int32_t line_number;		

  //NOTE: power!=power_per_unit_area
  void set_power(double power, double thickness);
  void set_max_power(double power);
  int32_t id_;	//number of the floorplan unit
  string name_;
  double width_;
  double height_;
  double leftx_;
  double bottomy_;
  double area_;
  double chip_density_;
  double interconnect_density_;
  vector<double>      model_unit_percentages_; //this is a list of the percentages of each of the dependent model units
  double power_;
  double power_per_unit_area_; //this is the current power dissipated by the flp (W/m^3)
  double max_power_per_unit_area_; //this is the max power density
  double max_dynamic_power_;
  double leakage_power_;
  double dynamic_power_;
  double hot_spot_count_;
  double hot_spot_duration_;
  double temperature_;
  sesctherm3Ddatalibrary* datalibrary_;
  vector<vector<model_unit*> > located_units_;
};

/* This encapsulates the entire chip floorplan, this includes all three layers including the inner virtual layer */
class chip_floorplan {
public:
    chip_floorplan(sesctherm3Ddatalibrary* datalibrary);
    friend ostream& operator<<(ostream& os, chip_floorplan& tmp_floorplan);
    void get_floorplan_ucool(ifstream& if_infile, double width, double height);
    // read the .flp file, store information to chip_flp_units
    // store information to leftx_coords (sorted list of x-values)
    // store information to bottom_corrds (corted list of y-values)
    void get_floorplan(bool get_from_sescconf, int32_t flp_num);	
	void get_floorplan_sescconf(int flp_num);
	void get_floorplan_flpfile();
    // This offsets the data values
    void offset_floorplan(double x_amount, double y_amount);
    /* total chip width	*/
    double get_total_width(void);

    /* total chip height */
    double get_total_height(void);
    double get_total_area(void);

    vector<chip_flp_unit> & get_flp_units(void);

    vector<chip_flp_unit> & get_leftx_coords(void);

    vector<chip_flp_unit> & get_bottomy_coords(void);

    bool is_floorplan_defined();

    string get_floorplan_name();


    bool flp_unit_exists(double x, double y);
    chip_flp_unit & find_flp_unit(double x, double y);
    vector<chip_flp_unit>     flp_units_;
    vector<double>     leftx_coords_;
    vector<double>     bottomy_coords_;
    double max_x_;
    double max_y_;
    double area_;
    double width_;
    double height_;
    double hot_spot_count_;
    double hot_spot_duration_;
    string floorplan_name_;
    bool floorplan_defined_;
	sesctherm3Ddatalibrary* datalibrary_;
};



#endif
