// Description: 3-Dimensional Thermal Model with Micro-cooler implementation
// Authors    : Joseph Nayfach - Battilana
//
//This file defines algorithms to draw either:
//1) The chip floorplan (output as fig file)


#ifndef SESCTHERM3D_GRAPHICS_H
#define SESCTHERM3D_GRAPHICS_H

class sesctherm3Dgraphics;

class sesctherm3Dgraphics {
public:
	
	sesctherm3Dgraphics(sesctherm3Ddatalibrary* datalibrary);
	void print_floorplans();
	void print_graphics_helper(sesctherm3Dgraphicsfiletype& graphics_file_type,
							   std::vector<double>& temperature_map,
								std::vector<double>& power_map,
								dynamic_array<model_unit>& layer_dyn,
							   int32_t layer);
	void print_graphics();
	void data_flp2svg(std::vector<chip_flp_unit>& flp_units, std::vector<double>& data_map, double min_dval, double max_dval, std::ofstream& of_graphics_outfile);
	void data_model2svg(vector<chip_flp_unit>& flp_units, dynamic_array<model_unit>& layer_dyn, int32_t data_type, int32_t sample_type, double min_dval, double max_dval, std::ofstream& of_graphics_outfile);
	void flp2svg(vector<chip_flp_unit>& flp_units,vector<double> data_map, std::ofstream& of_graphics_outfile, double& min_dataval, double& max_dataval);
	void flp2svg_simple(vector<chip_flp_unit>& flp_units, std::ofstream& of_graphics_outfile);
	void model2svg_helper(std::vector<chip_flp_unit>& flp_units, 
											   dynamic_array<model_unit>& layer_dyn, 
											   int32_t data_type,
												int sample_type,
											   std::ofstream& of_graphics_outfile, double& min_dataval, double& max_dataval);
	void calc_x_bound(std::vector<chip_flp_unit>& chip_flp_units, double& min_x, double& max_x, double& min_y, double& max_y);
	void calc_x_bound(dynamic_array<model_unit>& chip_flp_units, double& min_x, double& max_x, double& min_y, double& max_y);
	void draw_color_scale_bar(double max_dataval, double min_dataval, double max_x, double max_y, double min_x, double min_y, std::ofstream& of_graphics_outfile);
	void create_open_graphics_file(int data_type, int32_t unit_type, int32_t sample_type, int32_t layer_computation, std::vector<int>& layers, int32_t layer, std::ofstream& of_graphics_outfile);
	

	sesctherm3Ddatalibrary* datalibrary_;
	double num_levels_;			    //number of colors used
	double max_rotate_;				//maximum hue rotation  								 
	double stroke_coeff_;			//used to tune the stroke-width
	double stroke_opacity_;			//used to control the opacity of the floor plan
	double smallest_shown_;			//fraction of the entire chip necessary to see macro
	double zoom_;
	double txt_offset_;
	//static int32_t palette_[21][3];		
	
	vector<sesctherm3Dgraphicscolor> colors_;	//these are the colors to be used in the palette
	vector<sesctherm3Dgraphicscolor> palette_;	//this is the actual palette
	bool floorplans_printed_;
};

class sesctherm3Dgraphicsfiletype{
public:
	sesctherm3Dgraphicsfiletype(string config_string);
	int layer_computation_; //GFX_NORMAL/GFX_AVE/GFX_DIF
	int unit_type_;			//GFX_MUNIT/GFX_FUNIT
	int data_type_;			//GFX_POWER/GFX_TEMP
	int sample_type_;		//GFX_CUR/GFX_MAX/GFX_MIN/GFX_AVE
	double min_val_;
	double max_val_;
	std::vector<int> layers_;	//layers used
	friend ostream& operator<<(ostream& out, const sesctherm3Dgraphicsfiletype& type)
	{
		out << "printing sesctherm3Dgraphicsfiletype object" << endl;
		out << "Layer_computation_ :" << type.layer_computation_ << endl;
		out << "Unit_type_: " << type.unit_type_ << endl;
		out << "Data_type_: " << type.data_type_ << endl;
		out << "Sample_type_: " << type.sample_type_ << endl;
		out << "Layers_:[";
		for(uint32_t i=0;i< type.layers_.size();i++)
			out << type.layers_[i] << " ";
		out << "]" << endl;
		return out;
	}
	
};

class sesctherm3Dgraphicscolor{
public:
	sesctherm3Dgraphicscolor(int r, int32_t g, int32_t b, int32_t steps);
	static void create_palette(vector<sesctherm3Dgraphicscolor>& palette,vector<sesctherm3Dgraphicscolor>& colors);
	static void gradient(vector<sesctherm3Dgraphicscolor>& palette, sesctherm3Dgraphicscolor& color1, sesctherm3Dgraphicscolor& color2, int32_t steps);
	int r_;
	int g_;
	int b_;
	int steps_;
};

#endif

