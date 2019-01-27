//----------------------------------------------------------------------------
// File: sesctherm3Dgraphics.cpp
//
// Description: graphics subsystem to output svg file of either temperature or power map grouped as floorplan units or model units
// Authors    : Joseph Nayfach - Battilana
//



#include "sesctherm3Dinclude.h"

using namespace std;
using namespace mtl;

sesctherm3Dgraphicscolor::sesctherm3Dgraphicscolor(int r, int32_t g, int32_t b, int32_t steps){
	r_=r;
	g_=g;
	b_=b;
	steps_=steps;
}

void sesctherm3Dgraphicscolor::create_palette(vector<sesctherm3Dgraphicscolor>& palette, vector<sesctherm3Dgraphicscolor>& colors){
	if(!palette.empty()) palette.clear();
	for (uint32_t i=0; i<colors.size()-1; i++) {
		sesctherm3Dgraphicscolor::gradient(palette, colors[i], colors[i+1], colors[i].steps_);
	}
}

void sesctherm3Dgraphicscolor::gradient(vector<sesctherm3Dgraphicscolor>& palette, sesctherm3Dgraphicscolor& color1, sesctherm3Dgraphicscolor& color2, int32_t steps){
	int step_r=(int)((color1.r_ - color2.r_)/(steps-1));
	int step_g=(int)((color1.g_ - color2.g_)/(steps-1));
	int step_b=(int)((color1.b_ - color2.b_)/(steps-1));
	for(int i=0;i<steps;i++){
		sesctherm3Dgraphicscolor newcolor( (int)round(color1.r_ - (step_r * i)),
										   (int)round(color1.g_ - (step_g * i)),
										   (int)round(color1.b_ - (step_b * i)),
										   0
										   );
		palette.push_back(newcolor);
	}
}



sesctherm3Dgraphicsfiletype::sesctherm3Dgraphicsfiletype(string config_string){
    std::vector<string> tokens,tokens2;	
	
	layer_computation_=-1;
	unit_type_=-1;
	sample_type_=-1;
	data_type_=-1;
	
	sesctherm_utilities::Tokenize(config_string, tokens, "_, "); //tokenize by underscore and space and comma
	
	/*cerr <<  "printing tokens:" << endl;
	for (uint32_t i=0;i<tokens.size();i++){
		cerr <<  tokens[i] << ",";
	}
	cerr << endl;
	*/

	sesctherm_utilities::Tokenize(tokens[4], tokens2, "[]-");
	
	min_val_=sesctherm_utilities::convertToDouble(tokens2[0]);
	max_val_=sesctherm_utilities::convertToDouble(tokens2[1]);

	
	for(uint32_t i=5;i<tokens.size();i++){
		layers_.push_back(sesctherm_utilities::convertToInt(tokens[i]));
	}

	if(!strcmp(tokens[0].c_str(),"FLOORPLAN")){
		layer_computation_=GFX_LAYER_FLOORPLAN;
		return;
	}
	
	const char* layer_computation=tokens[0].c_str();
	const char* data_type=tokens[1].c_str();
	const char* sample_type=tokens[2].c_str();
	const char* unit_type=tokens[3].c_str(); 
	
	
	//GFX_LAYER_NORMAL/GFX_LAYER_AVE/GFX_LAYER_DIF
	if(!strcmp(layer_computation, "NORM")){
		layer_computation_=GFX_LAYER_NORMAL;
	}
	else if(!strcmp(layer_computation, "AVE")){
		layer_computation_=GFX_LAYER_AVE;
	}
	else if(!strcmp(layer_computation, "DIF")){
		layer_computation_=GFX_LAYER_DIF;
	}
	else{
		cerr << "FATAL: graphics file type [" << config_string << "] specified in config file is invalid" << endl;
		exit(1);
	}
	
	//GFX_POWER/GFX_TEMP
	if(!strcmp(data_type, "POWER")){
		data_type_=GFX_POWER;
	}
	else if(!strcmp(data_type, "TEMP")){	   
		data_type_=GFX_TEMP;
	}
	else{ 	
		cerr << "FATAL: graphics file type [" << config_string << "] specified in config file is invalid" << endl;
		exit(1);
	}
	
	
	if(!strcmp(sample_type,"CUR")){
		sample_type_=GFX_CUR;
	}
	else if(!strcmp(sample_type,"MAX")){
		sample_type_=GFX_MAX;		
	}
	else if(!strcmp(sample_type,"MIN")){
		sample_type_=GFX_MIN;		
	}
	else if(!strcmp(sample_type,"AVE")){
		sample_type_=GFX_AVE;		
	}
	else{
		cerr << "FATAL: graphics file type [" << config_string << "] specified in config file is invalid" << endl;
		exit(1);
	}
	
	if(!strcmp(unit_type,"M")){
		unit_type_=GFX_MUNIT;
	}
	else if(!strcmp(unit_type,"F")){
		unit_type_=GFX_FUNIT;
		
	}
	else{
		cerr << "FATAL: graphics file type [" << config_string << "] specified in config file is invalid" << endl;
		exit(1);
	}
}


/*int sesctherm3Dgraphics::palette_[21][3] = {	
	{255,0,0}, 
	{255,51,0},
	{255,102,0},
	{255,153,0},
	{255,204,0},
	{255,255,0},
	{204,255,0},
	{153,255,0},
	{102,255,0},
	{51,255,0},
	{0,255,0},
	{0,255,51},
	{0,255,102},
	{0,255,153},
	{0,255,204},
	{0,255,255},
	{0,204,255},
	{0,153,255},
	{0,102,255},
	{0,51,255},
	{0,0,255}
};
*/



//output a svg file based on floorplan
sesctherm3Dgraphics::sesctherm3Dgraphics(sesctherm3Ddatalibrary* datalibrary){
	datalibrary_=datalibrary;
	num_levels_=260;					//number of colors used
	max_rotate_=200;				//maximum hue rotation  								 
	stroke_coeff_=pow(10.0,-7);		//used to tune the stroke-width
	stroke_opacity_=0;				//used to control the opacity of the floor plan
	smallest_shown_=10000;			//fraction of the entire chip necessary to see macro
	zoom_=pow(10.0,6);
	txt_offset_=100;
	floorplans_printed_=false;
	sesctherm3Dgraphicscolor color1(255,0,0, 120);	//blue-green, 12 steps
	sesctherm3Dgraphicscolor color2(255,255,0, 50);  //green-yellow, 5 steps
	sesctherm3Dgraphicscolor color3(0,255,255, 90);	//yellow-red, 9 steps
	sesctherm3Dgraphicscolor color4(0,0,255, 0);	//	
	colors_.push_back(color1);
	colors_.push_back(color2);
	colors_.push_back(color3);
	colors_.push_back(color4);
	sesctherm3Dgraphicscolor::create_palette(palette_,colors_);	//store the palette based upon the colors
}

void sesctherm3Dgraphics::print_floorplans(){
	if(floorplans_printed_ == true)
		return;
	
	for(uint32_t i=0;i<datalibrary_->config_data_->graphics_file_types_.size();i++){
		
		//if we haven't printed out the floorplan files yet, then print them
		if(datalibrary_->config_data_->graphics_file_types_[i]->layer_computation_==GFX_LAYER_FLOORPLAN){
#ifdef _SESCTHERM_DEBUG
//				cerr << datalibrary_->config_data_->graphics_file_types_[i] << endl;
#endif
			for(uint32_t j=0;j<datalibrary_->config_data_->graphics_file_types_[i]->layers_.size();j++){
				int layer_num=datalibrary_->config_data_->graphics_file_types_[i]->layers_[j];
				std::ofstream of_graphics_outfile;
				create_open_graphics_file(0, 0, 0, GFX_LAYER_FLOORPLAN, datalibrary_->config_data_->graphics_file_types_[i]->layers_, layer_num, of_graphics_outfile);
				flp2svg_simple(datalibrary_->all_layers_info_[layer_num]->chip_floorplan_->flp_units_, of_graphics_outfile);			//print a floorplan file for layer i
				of_graphics_outfile.close();
			}
			
		}
	}
	floorplans_printed_==true;
}



void sesctherm3Dgraphics::print_graphics_helper(sesctherm3Dgraphicsfiletype& graphics_file_type,
												std::vector<double>& temperature_map,
												std::vector<double>& power_map,
												dynamic_array<model_unit>& layer_dyn,
												int layer){
#ifdef _SESCTHERM_DEBUG
//	cerr << graphics_file_type << endl;
#endif
	int unit_type=graphics_file_type.unit_type_;
	int data_type=graphics_file_type.data_type_;
	int sample_type=graphics_file_type.sample_type_;
	int layer_computation=graphics_file_type.layer_computation_;
	double min_data_val = graphics_file_type.min_val_;
	double max_data_val = graphics_file_type.max_val_;
	int graphics_floorplan_layer=datalibrary_->config_data_->graphics_floorplan_layer_;
	if(graphics_floorplan_layer < 0 || (uint32_t)graphics_floorplan_layer > datalibrary_->all_layers_info_.size() || datalibrary_->all_layers_info_[graphics_floorplan_layer]->chip_floorplan_==NULL){
		cerr << "sesctherm3Dgraphics::print_graphics_helper => graphics_floorplan_layer is invalid in configuration file (no chip floorplan has been defined for layer " << graphics_floorplan_layer << ")" << endl;
		exit(1);
	}

		
	std::vector<int>& layers=graphics_file_type.layers_;
	std::vector<chip_flp_unit>& flp_units=datalibrary_->all_layers_info_[graphics_floorplan_layer]->chip_floorplan_->flp_units_;
	
	//create and open graphics file
	std::ofstream of_graphics_outfile;
	create_open_graphics_file(data_type, unit_type, sample_type, layer_computation, layers, layer, of_graphics_outfile);
	
	if(unit_type==GFX_FUNIT){
		//print the colorized floorplan
		if(data_type==GFX_TEMP){
		  temperature_map = model_unit::compute_average_temps(graphics_floorplan_layer, layer_dyn, sample_type, datalibrary_);
		  data_flp2svg(flp_units,temperature_map, min_data_val, max_data_val, of_graphics_outfile);
		}
		else{
			power_map=model_unit::compute_average_powers(graphics_floorplan_layer, layer_dyn, sample_type, datalibrary_);
			data_flp2svg(flp_units,power_map, min_data_val, max_data_val, of_graphics_outfile);
		}
	}
	//otherwise we print out the layer by model-unit
	else{

		//we print out the data specified by data_type that is stored in the layer_dyn
		data_model2svg(flp_units,layer_dyn, data_type, sample_type, min_data_val, max_data_val, of_graphics_outfile);
		
	}
	of_graphics_outfile.close();
	
}

//it is assumed that print_graphics will be called at the end of a given sampling period if
//the average/max/min values are desired. Otherwise, it will get old values.
void sesctherm3Dgraphics::print_graphics(){
	vector<sesctherm3Dgraphicsfiletype*>& graphics_file_types =datalibrary_->config_data_->graphics_file_types_;
	int graphics_floorplan_layer=datalibrary_->config_data_->graphics_floorplan_layer_;
	if(datalibrary_->all_layers_info_[graphics_floorplan_layer]->chip_floorplan_==NULL){
		cerr << "FATAL: sesctherm3Dgraphics::print_graphics() => graphics_floorplan_layer is invalid in configuration file (no chip floorplan exists for this layer)" << endl;
		exit(1);
	}
		
	dynamic_array<model_unit> graphics_layer_dyn(datalibrary_);			//this will hold all of the relevent data for output
	std::vector<double> power_map;
	std::vector<double> temperature_map;
	int layer_num=0;
	int floorplan_layer = datalibrary_->config_data_->graphics_floorplan_layer_;
			
	print_floorplans();
	
	for(uint32_t i=0;i<graphics_file_types.size();i++){
		if(graphics_file_types[i]->layer_computation_==GFX_LAYER_FLOORPLAN)
			continue;			//we have already printed the floorplan-type files, no need to do it again

		graphics_layer_dyn.clear();
		if(!temperature_map.empty())
			temperature_map.clear();
		if(!power_map.empty())
			power_map.clear();
		
		switch(graphics_file_types[i]->layer_computation_){			
			case GFX_LAYER_NORMAL:
				//this means that we print out one graphics file per layer specified
				for(uint32_t j=0;j<graphics_file_types[i]->layers_.size();j++){
					int layer_num=graphics_file_types[i]->layers_[j];
					if(layer_num < 0 || (uint32_t)layer_num > datalibrary_->all_layers_info_.size()){
						cerr << "Attempting to print graphics for invalid layer: " << layer_num << endl;
						exit(1);
					}
					for (uint32_t itor_x=0;itor_x<datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_->ncols();itor_x++) {
						for (uint32_t itor_y=0;itor_y<datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_->nrows();itor_y++) {
							if ((*datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_)[itor_x][itor_y].defined_==false)
								(*datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_)[itor_x][itor_y].sample_temperature_=0;
							else
								(*datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_)[itor_x][itor_y].sample_temperature_=
									*(*datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_)[itor_x][itor_y].get_temperature();
						}
					}
					//pass the floorplan_layer_dyn to the graphics helper
					print_graphics_helper(*graphics_file_types[i],
										  temperature_map,
										  power_map,
										  *datalibrary_->all_layers_info_[layer_num]->floorplan_layer_dyn_,
										  layer_num);
				}
				break;
			case GFX_LAYER_AVE:
				//this means that we average the data across all the layers for the particular data_type
				graphics_layer_dyn=sesctherm3Dlayerinfo::compute_layer_averages(graphics_file_types[i]->layers_,datalibrary_);					
				layer_num=0;
				print_graphics_helper(*graphics_file_types[i],
									  temperature_map,
									  power_map,
									  graphics_layer_dyn,
									  layer_num);
				break;
			case GFX_LAYER_DIF:
				layer_num=0;
				graphics_layer_dyn=sesctherm3Dlayerinfo::compute_layer_diff(graphics_file_types[i]->layers_[0], 
																			graphics_file_types[i]->layers_[1], datalibrary_);					
				
				print_graphics_helper(*graphics_file_types[i],
									  temperature_map,
									  power_map,
									  graphics_layer_dyn,
									  layer_num);
				
				break;
			default:
				cerr << "sesctherm3Dgraphics::printGraphics => graphics_file_types[i]->layer_computation_ is invalid" << endl;
				exit(1);
		}
	}
}

//print out a colorized version of the floorplan for a given dataset (temperature/power)
void sesctherm3Dgraphics::data_flp2svg(std::vector<chip_flp_unit>& flp_units, std::vector<double>& data_map, double min_dval, double max_dval, std::ofstream& of_graphics_outfile){		
	
	double min_x=0;
	double max_x=0;
	double min_y=0;
	double max_y=0;	
	double x_bound=0;
	

	
	calc_x_bound(flp_units, min_x, max_x, min_y, max_y);
	

#ifdef _SESCTHERM_DEBUG
//	cerr << "DATA_FLP2SVG: minx:" <<  min_x << "maxx:" << max_x << "miny:" << min_y << "maxy:" << max_y << endl;
#endif
	x_bound=max_x*1.2;
	(of_graphics_outfile) << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\" " << endl;
	(of_graphics_outfile) << "\"http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd\"> " << endl;
	(of_graphics_outfile) << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"" << 
										datalibrary_->config_data_->resolution_x_ << "\" height=\"" << 
										datalibrary_->config_data_->resolution_y_ << "\" " << endl;
	if(datalibrary_->config_data_->enable_perspective_view_){
	//	(of_graphics_outfile) << "\t viewBox=\"" << min_x*1.3 << " " << min_y*1.1 << " " << .5*sqrtf(powf(max_x-minx_x,2.0)+powf(max_y-min_y,2.0)) << " " << (max_y-min_y)*1.5 << "\"> " << endl;
	(of_graphics_outfile) << "\t viewBox=\"" << min_x*.93 << " " << min_y*.98 << " " << sqrtf(powf(max_x-min_x,2.0)+powf(max_y-min_y,2.0))*1.5 << " " << sqrtf(powf(max_x-min_x,2.0)+powf(max_y-min_y,2.0))*1.3 << "\"> " << endl;
	}
	else{
		(of_graphics_outfile) << "\t viewBox=\"" << min_x*.98 << " " << min_y*.98 << " " << (max_x-min_x)*1.5 << " " << (max_y-min_y)*1.5 << "\"> " << endl;	
	}
	
	(of_graphics_outfile) << "<title>Sesctherm</title>" << endl;
	
	(of_graphics_outfile) << "<defs>" << endl;
	
	(of_graphics_outfile) << "<g id=\"chip_flp_unit_names\" style=\"stroke: none\">" << endl;
	
	
	//print the floorplan shared based upon the data_map
	
	
	flp2svg_simple(flp_units,of_graphics_outfile);
	(of_graphics_outfile) << "</g>";
	
	(of_graphics_outfile) << "<g id=\"floorplan\" style=\"stroke: none; fill: red\">" << endl;
	
	flp2svg(flp_units, data_map, of_graphics_outfile, min_dval, max_dval);
		(of_graphics_outfile) << "</g>";
		
		(of_graphics_outfile) << "<g id=\"thermal_scale\" style=\"stroke: none; fill: red\">" << endl;
	draw_color_scale_bar(max_dval,min_dval,max_x,max_y,min_x,max_y, of_graphics_outfile);
	
	(of_graphics_outfile) << "</g>";
	
	(of_graphics_outfile) << "</defs>";
	if(datalibrary_->config_data_->enable_perspective_view_){
		(of_graphics_outfile) << "<use id=\"flp3\" xlink:href=\"#floorplan\" transform=\" translate(" << min_x << "," << min_y << ") scale(1.5,0.9) skewX(35) rotate(-35) translate(" << -1.1*min_x << "," << -1.005*min_y << ")\"/>" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#chip_flp_unit_names\" transform=\" translate(" << min_x << "," << min_y << ") scale(1.5,0.9) skewX(35) rotate(-35) translate(" << -1.1*min_x << "," << -1.005*min_y << ")\"/>" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#thermal_scale\"/>" << endl;
		(of_graphics_outfile) << "</svg>" << endl;
	}
	else{
		
		(of_graphics_outfile) << "<use id=\"flp3\" xlink:href=\"#floorplan\" />" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#chip_flp_unit_names\" />" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#thermal_scale\"/>" << endl;
		(of_graphics_outfile) << "</svg>" << endl;		
		
	}
	
}

//print out a colorized version of the model /w floorplan overlay for a given dataset (temperature/power)
void sesctherm3Dgraphics::data_model2svg(vector<chip_flp_unit>& flp_units, dynamic_array<model_unit>& layer_dyn, int32_t data_type, int32_t sample_type, double min_dval, double max_dval, std::ofstream& of_graphics_outfile){	
	
	double min_x=0;
	double max_x=0;
	double min_y=0;
	double max_y=0;	
	double x_bound=0;

	calc_x_bound(layer_dyn, min_x, max_x, min_y, max_y);
	
#ifdef _SESCTHERM_DEBUG
//	cerr << "DATA_MODEL2SVG: minx:" <<  min_x << "maxx:" << max_x << "miny:" << min_y << "maxy:" << max_y << endl;
#endif
	x_bound=max_x*1.2;
	
	
	(of_graphics_outfile) << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\" " << endl;
	(of_graphics_outfile) << "\"http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd\"> " << endl;
	(of_graphics_outfile) << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"" << 
		datalibrary_->config_data_->resolution_x_ << "\" height=\"" << 
		datalibrary_->config_data_->resolution_y_ << "\" " << endl;
	if(datalibrary_->config_data_->enable_perspective_view_){
//		(of_graphics_outfile) << "\t viewBox=\"" << min_x*1.3 << " " << min_y*1.1 << " " << .5*sqrtf(powf(x_bound,2.0)+powf(max_y,2.0)) << " " << max_y << "\"> " << endl;
	(of_graphics_outfile) << "\t viewBox=\"" << min_x*.93 << " " << min_y*.98 << " " << sqrtf(powf(max_x-min_x,2.0)+powf(max_y-min_y,2.0))*1.5 << " " << sqrtf(powf(max_x-min_x,2.0)+powf(max_y-min_y,2.0))*1.3 << "\"> " << endl;
	}
	else{
		(of_graphics_outfile) << "\t viewBox=\"" << min_x*.98 << " " << min_y*.98 << " " << (max_x-min_x)*1.5 << " " << (max_y-min_y)*1.5 << "\"> " << endl;	
	}
	
	
	
	(of_graphics_outfile) << "<title>Sesctherm</title>" << endl;
	
	(of_graphics_outfile) << "<defs>" << endl;
	
	(of_graphics_outfile) << "<g id=\"chip_flp_unit_names\" style=\"stroke: none\">" << endl;
	

			
	flp2svg_simple(flp_units, of_graphics_outfile);	
	

			
	(of_graphics_outfile) << "</g>";
	
	(of_graphics_outfile) << "<g id=\"floorplan\" style=\"stroke: none; fill: red\">" << endl;


	
	model2svg_helper(flp_units, layer_dyn, data_type, sample_type, of_graphics_outfile, min_dval, max_dval);
	

	(of_graphics_outfile) << "</g>";
	
	(of_graphics_outfile) << "<g id=\"thermal_scale\" style=\"stroke: none; fill: red\">" << endl;


	
	draw_color_scale_bar(max_dval,min_dval,max_x,max_y,min_x,min_y,of_graphics_outfile);
	(of_graphics_outfile) << "</g>";
	
	(of_graphics_outfile) << "</defs>";
	
	if(datalibrary_->config_data_->enable_perspective_view_){
		(of_graphics_outfile) << "<use id=\"flp3\" xlink:href=\"#floorplan\" transform=\" translate(" << min_x << "," << min_y << ") scale(1.5,0.9) skewX(35) rotate(-35) translate(" << -1.1*min_x << "," << -1.005*min_y << ")\"/>" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#chip_flp_unit_names\" transform=\" translate(" << min_x << "," << min_y << ") scale(1.5,0.9) skewX(35) rotate(-35) translate(" << -1.1*min_x << "," << -1.005*min_y << ")\"/>" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#thermal_scale\"/>" << endl;
		(of_graphics_outfile) << "</svg>" << endl;
	}
	else{
		
		(of_graphics_outfile) << "<use id=\"flp3\" xlink:href=\"#floorplan\" />" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#chip_flp_unit_names\" />" << endl;
		(of_graphics_outfile) << "<use xlink:href=\"#thermal_scale\"/>" << endl;
		(of_graphics_outfile) << "</svg>" << endl;		
		
	}
	
}

void sesctherm3Dgraphics::flp2svg(vector<chip_flp_unit>& flp_units,vector<double> data_map, std::ofstream& of_graphics_outfile, double& max_dataval, double& min_dataval){
	int level=0;
	double current_dataval=0;
	if(min_dataval==-1)
		 min_dataval=pow(10.0,10);
	//no need for this (it will be -1 anyway) max_dataval=-1;
	I(data_map.size()==flp_units.size());	//ensure that the data_map array is the same size as the flp_units array
											//get max data value, min data value
	for (uint32_t i=0;i<data_map.size();i++) {
		if(min_dataval==-1)
			min_dataval=MIN(min_dataval,data_map[i]);
		if(max_dataval==-1)
			max_dataval=MAX(max_dataval,data_map[i]);
	}
	
	double x_min=0;
	double y_min=0;
	uint32_t x_itor=0;
	uint32_t y_itor=0;
	for (uint32_t i=0;i<data_map.size();i++) {
		current_dataval=data_map[i];            					
		level =(int)((max_dataval-current_dataval)/((max_dataval-min_dataval)*(num_levels_-1)));

		(of_graphics_outfile) << "\t" << "<rect x=\"" << flp_units[i].leftx_*zoom_ << "\" y=\"" << flp_units[i].bottomy_*zoom_ << "\"";
		(of_graphics_outfile) << " width=\"" << flp_units[i].width_*zoom_ << "\"";
		(of_graphics_outfile) << " height=\"" << flp_units[i].height_*zoom_ << "\"";
		(of_graphics_outfile) << " style=\"fill:rgb(" << 
			sesctherm_utilities::stringify(palette_[level].r_) << "," << sesctherm_utilities::stringify(palette_[level].g_) << "," << sesctherm_utilities::stringify(palette_[level].b_) << ")\" />" << endl;
	}

}

void sesctherm3Dgraphics::flp2svg_simple(vector<chip_flp_unit>& flp_units, std::ofstream& of_graphics_outfile){
	double max_t=0;
	double min_x=pow(10.0,10);
	double min_y=pow(10.0,10);
	double max_x=0;
	double max_y=0;
	double tot_x=0;
	double tot_y=0;
	double x_bound;
	double in_minx, in_miny, in_maxx, in_maxy;
	
	calc_x_bound(flp_units, min_x, max_x, min_y, max_y);
#ifdef _SESCTHERM_DEBUG
//	cerr << "FLP2SVG: minx:" <<  min_x << "maxx:" << max_x << "miny:" << min_y << "maxy:" << max_y << endl;
#endif
	tot_x=max_x-min_x;
	tot_y=max_y-min_y;
	x_bound=max_x*1.2;
	
	double w,h,start_y,end_y,txt_start_x,txt_start_y;
	for(uint32_t i=0;i<flp_units.size();i++){
		double w=flp_units[i].width_;
		double h=flp_units[i].height_;
		double txt_start_x = flp_units[i].leftx_;
		double txt_start_y = flp_units[i].bottomy_+flp_units[i].height_;
		
			//draw leftx,bottomy to rightx,bottomy
			(of_graphics_outfile) << " \t" << "<line x1=\" " << flp_units[i].leftx_*zoom_ << " \" y1=\" " << flp_units[i].bottomy_*zoom_ << "\" ";
			(of_graphics_outfile) << "x2=\" " << (flp_units[i].leftx_+flp_units[i].width_)*zoom_ << " \" y2=\" " << flp_units[i].bottomy_*zoom_<< "\" ";
			(of_graphics_outfile) << " style=\"stroke:black;stroke-width:30\" />" << endl;
			
			//draw rightx,bottomy to rightx,topy
			(of_graphics_outfile) << " \t" <<"<line x1=\" " << (flp_units[i].leftx_+flp_units[i].width_)*zoom_ << " \" y1=\" " << flp_units[i].bottomy_*zoom_ << "\" ";
			(of_graphics_outfile) << " x2=\" " << (flp_units[i].leftx_+flp_units[i].width_)*zoom_ << " \" y2=\" " << (flp_units[i].bottomy_+flp_units[i].height_)*zoom_ << "\" ";
			(of_graphics_outfile) << " style=\"stroke:black;stroke-width:30\" />" << endl;
			
			//draw rightx,topy to leftx,topy
			(of_graphics_outfile) << " \t" <<"<line x1=\" " << (flp_units[i].leftx_+flp_units[i].width_)*zoom_ << " \" y1=\" " << (flp_units[i].bottomy_+flp_units[i].height_)*zoom_ << "\" ";
			(of_graphics_outfile) << " x2=\" " << flp_units[i].leftx_*zoom_ << " \" y2=\" " << (flp_units[i].bottomy_+flp_units[i].height_)*zoom_ << "\" ";
			(of_graphics_outfile) << " style=\"stroke:black;stroke-width:30\" />" << endl;
			
			//draw leftx,topy to leftx,bottomy
			(of_graphics_outfile) << " \t" <<"<line x1=\" " << flp_units[i].leftx_*zoom_ << " \" y1=\" " << (flp_units[i].bottomy_+flp_units[i].height_)*zoom_ << "\" ";
			(of_graphics_outfile) << " x2=\" " << flp_units[i].leftx_*zoom_ << " \" y2=\" " << flp_units[i].bottomy_*zoom_<< "\" ";
			(of_graphics_outfile) << " style=\"stroke:black;stroke-width:30\" />" << endl;
			
			(of_graphics_outfile) << "\t" << "<text x=\"" << txt_start_x*zoom_ << " \" y=\"" << txt_start_y*zoom_ << "\"";
			(of_graphics_outfile) << " fill=\"black\" text_anchor=\"start\" font-size=\"300\" style=\"font-size:180\" >";
			(of_graphics_outfile) << flp_units[i].name_ << "</text>" << endl;
	}
	
}


void sesctherm3Dgraphics::model2svg_helper(std::vector<chip_flp_unit>& flp_units, 
										   dynamic_array<model_unit>& layer_dyn, 
										   int32_t data_type,
										   int32_t sample_type,
										   std::ofstream& of_graphics_outfile,
										   double& min_dataval,
										   double& max_dataval){
	int level=0;
	double current_dataval=0;
	if(min_dataval==-1)
		 min_dataval=pow(10.0,20);
	 //max_dataval=-1; //no need for this (it will be -1 anyway) 
	
	//get max data value, min data value
	for (uint32_t y_itor=0;y_itor<layer_dyn.nrows();y_itor++) {
        for (uint32_t x_itor=0;x_itor<layer_dyn.ncols();x_itor++) {
			if(data_type==GFX_POWER){
				switch(sample_type){
					case GFX_CUR:
						current_dataval=layer_dyn[x_itor][y_itor].energy_data_;            		
						break;
					case GFX_MIN:
						current_dataval=layer_dyn[x_itor][y_itor].min_energy_data_;
						break;
					case GFX_MAX:
						current_dataval=layer_dyn[x_itor][y_itor].max_energy_data_;            		
						break;
					case GFX_AVE:
						current_dataval=layer_dyn[x_itor][y_itor].ave_energy_data_;            		
						break;
					default:
						break;
				}
			}
			else if(data_type==GFX_TEMP){
				switch(sample_type){
					case GFX_CUR:
						current_dataval=layer_dyn[x_itor][y_itor].sample_temperature_;            		
						break;
					case GFX_MIN:
						current_dataval=layer_dyn[x_itor][y_itor].min_temperature_;
						break;
					case GFX_MAX:
						current_dataval=layer_dyn[x_itor][y_itor].max_temperature_;            		
						break;
					case GFX_AVE:
						current_dataval=layer_dyn[x_itor][y_itor].ave_temperature_;            		
						break;
					default:
						break;
				}
			}
			else{
				cerr << "sesctherm3Dgraphics::model2svg_helper() => invalid data type" << endl;
				exit(1);
			}
			if(min_dataval==-1)
				min_dataval=MIN(min_dataval,current_dataval);
			if(max_dataval==-1)
				max_dataval=MAX(max_dataval,current_dataval);
        }
	}
	
	double x_min=0;
	double y_min=0;
	uint32_t x_itor=0;
	uint32_t y_itor=0;
	

	
	for (y_itor=0;y_itor<layer_dyn.nrows();y_itor++) {
        for (x_itor=0;x_itor<layer_dyn.ncols();x_itor++) {
			if(data_type==GFX_POWER){
				switch(sample_type){
					case GFX_CUR:
						current_dataval=layer_dyn[x_itor][y_itor].energy_data_;            		
						break;
					case GFX_MIN:
						current_dataval=layer_dyn[x_itor][y_itor].min_energy_data_;
						break;
					case GFX_MAX:
						current_dataval=layer_dyn[x_itor][y_itor].max_energy_data_;            		
						break;
					case GFX_AVE:
						current_dataval=layer_dyn[x_itor][y_itor].ave_energy_data_;            		
						break;
					default:
						break;
				}
			}
			else if(data_type==GFX_TEMP){
				switch(sample_type){
					case GFX_CUR:
						current_dataval=layer_dyn[x_itor][y_itor].sample_temperature_;            		
						break;
					case GFX_MIN:
						current_dataval=layer_dyn[x_itor][y_itor].min_temperature_;
						break;
					case GFX_MAX:
						current_dataval=layer_dyn[x_itor][y_itor].max_temperature_;            		
						break;
					case GFX_AVE:
						current_dataval=layer_dyn[x_itor][y_itor].ave_temperature_;            		
						break;
					default:
						break;
				}
			}
			else{
				cerr << "sesctherm3Dgraphics::model2svg_helper() => invalid data type" << endl;
				exit(1);
			}	
#ifdef _SESCTHERM_DEBUG
//			cerr << current_dataval << "[" << x_itor << "][" << y_itor << "]" << ",";
#endif
			level =(int)((max_dataval-current_dataval)/(max_dataval-min_dataval)*(num_levels_-1));
			(of_graphics_outfile) << "\t" << "<rect x=\"" << layer_dyn[x_itor][y_itor].leftx_*zoom_ << "\" y=\"" << layer_dyn[x_itor][y_itor].bottomy_*zoom_ << "\"";
			(of_graphics_outfile) << " width=\"" << layer_dyn[x_itor][y_itor].width_*zoom_ << "\"";
			(of_graphics_outfile) << " height=\"" << layer_dyn[x_itor][y_itor].height_*zoom_ << "\"";
			(of_graphics_outfile) << " style=\"fill:rgb("<< 
				sesctherm_utilities::stringify(palette_[level].r_) << "," << sesctherm_utilities::stringify(palette_[level].g_) << "," << sesctherm_utilities::stringify(palette_[level].b_) << ")\" />" << endl;
		}
#ifdef _SESCTHERM_DEBUG
//		cerr << endl;
#endif
	}
	
}


void sesctherm3Dgraphics::calc_x_bound(std::vector<chip_flp_unit>& chip_flp_units, double& min_x, double& max_x, double& min_y, double& max_y){	
	min_x=pow(10.0,10);
	min_y=pow(10.0,10);
	max_x=0;
	max_y=0;
	double in_minx, in_miny, in_maxx, in_maxy;
	
	//get in_minx, in_miny, in_maxx, in_maxy, tot_x, tot_y
	for(uint32_t i=0;i<chip_flp_units.size();i++){
		in_minx=chip_flp_units[i].leftx_*zoom_;
		in_miny=chip_flp_units[i].bottomy_*zoom_;
		in_maxx=(chip_flp_units[i].leftx_+
				 chip_flp_units[i].width_)*zoom_;
		in_maxy=(chip_flp_units[i].bottomy_+
				 chip_flp_units[i].height_)*zoom_;						
		min_x=MIN(min_x,in_minx);
		min_y=MIN(min_y,in_miny);
		max_x=MAX(max_x,in_maxx);
		max_y=MAX(max_y,in_maxy);	
	}
}

void sesctherm3Dgraphics::calc_x_bound(dynamic_array<model_unit>& dyn_layer, double& min_x, double& max_x, double& min_y, double& max_y){	
	min_x=pow(10.0,10);
	min_y=pow(10.0,10);
	max_x=0;
	max_y=0;
	double in_minx, in_miny, in_maxx, in_maxy;
	
	
	//get in_minx, in_miny, in_maxx, in_maxy, tot_x, tot_y
	for(uint32_t y_itor=0;y_itor<dyn_layer.nrows();y_itor++){
		for(uint32_t x_itor=0;x_itor<dyn_layer.ncols();x_itor++){
			if(!dyn_layer[x_itor][y_itor].defined_)
				continue;
			in_minx=dyn_layer[x_itor][y_itor].leftx_*zoom_;
			in_miny=dyn_layer[x_itor][y_itor].bottomy_*zoom_;
			in_maxx=(dyn_layer[x_itor][y_itor].leftx_+
					 dyn_layer[x_itor][y_itor].width_)*zoom_;
			in_maxy=(dyn_layer[x_itor][y_itor].bottomy_+
					 dyn_layer[x_itor][y_itor].height_)*zoom_;						
			min_x=MIN(min_x,in_minx);
			min_y=MIN(min_y,in_miny);
			max_x=MAX(max_x,in_maxx);
			max_y=MAX(max_y,in_maxy);	
		}		
	}
}

void sesctherm3Dgraphics::draw_color_scale_bar(double max_dataval, double min_dataval, double max_x, double max_y, double min_x, double min_y, std::ofstream& of_graphics_outfile){
	
	
	double txt_ymin=0;
	double w2=(max_x-min_x)*0.05;
	double h2=(max_y-min_y)/num_levels_;
	double clr_xmin=max_x*1.03;
	double clr_ymin=min_y;
	double scale_xmin=max_x*1.02;
	double scale_value;
	double final_scale_value=min_dataval+(1/num_levels_)*(max_dataval-min_dataval);
	double tot_x=max_x-min_x;
	double tot_y=max_y-min_y;
	
	//we want 15 data points
	for (int i=0; i<num_levels_; i++) {
			(of_graphics_outfile) << "\t" <<  "<rect x=\"" << clr_xmin << "\" y=\"" << clr_ymin << "\" width=\"" << w2 << "\" height=\"" << h2 << "\"";
			(of_graphics_outfile) << " style=\"fill:rgb(" << 
				sesctherm_utilities::stringify(palette_[i].r_) << "," << sesctherm_utilities::stringify(palette_[i].g_) << "," << sesctherm_utilities::stringify(palette_[i].b_) << "); stroke:none\" />" << endl;
			if (i%int(num_levels_/10)==0) {
				txt_ymin=clr_ymin+h2*0.5;
				scale_value=(max_dataval-min_dataval)*(1-i/(num_levels_-1))+min_dataval;
				(of_graphics_outfile) << "\t" << "<text x=\"" << scale_xmin << "\" y=\"" << txt_ymin << "\" fill=\"black\" text_anchor=\"start\" font-size=\"300\" style=\"font-size:250\">";
				(of_graphics_outfile) << scale_value << "</text>" << endl;
			}
		clr_ymin+=h2;
	}
	
	(of_graphics_outfile) << "\t" <<  "<text x=\"" << scale_xmin << "\" y=\"" << clr_ymin << "\" fill=\"black\" text_anchor=\"start\" font-size=\"300\" style=\"font-size:250\" >";
	(of_graphics_outfile) << min_dataval << "</text>" << endl;
	
}

//prefix is a prefix to place before the timestamp
//file format: prefix.timestamp.svg
//example layer0.001s.svg
//info_type: CUR_TEMP
//			 MAX_TEMP
//			 AVERAGE_TEMP
//			
//graphics file
void sesctherm3Dgraphics::create_open_graphics_file(int data_type, int32_t unit_type, int32_t sample_type, int32_t layer_computation, vector<int>& layers, int32_t layer, std::ofstream& of_graphics_outfile ){
	
	string filename="";
	
	switch(layer_computation){
		case GFX_LAYER_AVE: 
			filename += "[lcomp:AVE][layers:";
			//print layers used in average
			filename+= sesctherm_utilities::stringify(layers[0]);
			for(uint32_t i=1;i<layers.size();i++)
				filename+= "," + sesctherm_utilities::stringify(layers[i]);
				
				filename+="]";
			break;
		case GFX_LAYER_NORMAL:
			filename += "[lcomp:NORM][layer:" + sesctherm_utilities::stringify(layer) + "]";
			break;
		case GFX_LAYER_DIF:
			filename += "[lcomp:DIF][layers:";
			for(uint32_t i=1;i<layers.size();i++)
				filename+= "," +  sesctherm_utilities::stringify(layers[i]);
				
				filename+="]";
			break;
		case GFX_LAYER_FLOORPLAN:
			filename += "[lcomp:FLOORPLAN][layer:" + sesctherm_utilities::stringify(layer)  + "].svg";
			(of_graphics_outfile).open(filename.c_str(), std::ifstream::out);
			if (!(of_graphics_outfile)) {
				sesctherm_utilities::fatal("Cannot create graphics output (.svg) file" + filename + "\n");
			}
				return;
			
		default:
			cerr << "sesctherm3Dgraphics::create_open_graphics_file => invalid layer computation type" << endl;
			exit(1);
	}
				
				
	switch(sample_type){
		case GFX_CUR:
			filename+="[smpltype:CUR][" + sesctherm_utilities::stringify(datalibrary_->time_) + "s]";
			break;
		case GFX_AVE:
			filename+="[smpltype:AVE][" + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_start_time_) + "s to " + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_end_time_) + "s]";
			break;
		case GFX_MAX:
			filename+="[smpltype:MAX][" + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_start_time_) + "s to " + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_end_time_) + "s]";
			break;
		case GFX_MIN:
			filename+="[smpltype:MIN][" + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_start_time_) + "s to " + sesctherm_utilities::stringify(datalibrary_->config_data_->sample_end_time_) + "s]";
			break;
		default:
			cerr << "sesctherm3Dgraphics::create_open_graphics_file => invalid sample type" << endl;
			exit(1);
	}
	
	switch(unit_type){
		case GFX_FUNIT:
			filename+="[utype:FUNIT]";
			break;
		case GFX_MUNIT:
			filename+="[utype:MUNIT]";
			break;
		default:
			cerr << "sesctherm3Dgraphics::create_open_graphics_file => invalid unit type" << endl;
			exit(1);
			
	}
	
	switch(data_type){
		case GFX_POWER:
			filename+="[dtype:PWR]";
			break;
		case GFX_TEMP:
			filename+="[dtype:TEMP]";
			break;
		default:
			cerr << "sesctherm3Dgraphics::create_open_graphics_file => invalid unit type" << endl;
			exit(1);
			
	}
	
	filename+=".svg";
#ifdef _SESCTHERM_DEBUG
	cerr << "Attempting to Create Graphics Output File:" << filename << endl;
#endif
	
	(of_graphics_outfile).open(filename.c_str(), std::ifstream::out);
	if (!(of_graphics_outfile)) {
		sesctherm_utilities::fatal("Cannot create graphics output (.svg) file"+ filename + "\n");
	}
}

