#include "sesctherm3Dinclude.h"

//MATERIAL_LIST CLASS
void sesctherm3Dmaterial_list::create(string& name, double density, double spec_heat, double conductivity, double emissivity){
	sesctherm3Dmaterial *mat = new sesctherm3Dmaterial(name, density, spec_heat, conductivity, emissivity);
	
    if (storage.find(name) != storage.end()) {
        cerr << "Error: double insertion of same material" << name << endl;
        exit(1);
    }
	
    storage[name] = mat;
}

sesctherm3Dmaterial& sesctherm3Dmaterial_list::find(string name) {
    std::map<string, sesctherm3Dmaterial *>::iterator it = storage.find(name);
    if (it == storage.end()) {
        cerr << "Error: could not find material " << name << " within config file" << endl;
        exit(1);
    }
	
    return(*it->second);
}


//MATERIAL CLASS

sesctherm3Dmaterial::sesctherm3Dmaterial(string& name, double density, double spec_heat, double conductivity, double emissivity) {
    name_=name;
    density_=density;
    spec_heat_=spec_heat;
    conductivity_=conductivity;
	emissivity_=emissivity;
	
}


//Note: material properties change with temperature
//FIXME: we should add the transistor layer too!, this will change with temperature as well
//However, for this we need to find out (1) how BOX is effected by temperature (2) how silicon island is effected by temperature and 
// (3) effective capacitance is changed with temperature
 void sesctherm3Dmaterial::calc_material_properties_dyn_temp(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_){
		switch (datalibrary_->all_layers_info_[layer]->type_) {
			case BULK_SI_LAYER:
				sesctherm3Dmaterial::calc_silicon_layer_properties_dyn(layer, x,y,datalibrary_);
				break;
			default:
				break;
		}
}



//this updates the material properties of the particular dynarray element
//this is done to reflect the varying densities of the floorplan units
 void sesctherm3Dmaterial::calc_material_properties_dyn_density(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_){
	//there are two layers that are effected by density
	//these are the die_transistor_layer,  interconnect_layer
	switch (datalibrary_->all_layers_info_[layer]->type_) {
		case DIE_TRANSISTOR_LAYER:
			sesctherm3Dmaterial::calc_die_transistor_layer_properties_dyn(layer,x,y, datalibrary_);
			break;
		case INTERCONNECT_LAYER:
			sesctherm3Dmaterial::calc_interconnect_layer_properties_dyn(layer,x,y, datalibrary_);
			break;
			
		default:
			break;//otherwise don't recompute
	}
}


void sesctherm3Dmaterial::calc_silicon_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_){
			double temperature=*(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].get_temperature();
			//Silicon conductivity is temperature-dependent
			//117.5 - 0.42*(T-100) is what is typically used (FLOTHERM)
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_left_=117.5-0.42*(temperature-100);
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_right_=117.5-0.42*(temperature-100);
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_up_=117.5-0.42*(temperature-100);
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_down_=117.5-0.42*(temperature-100);
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_top_=117.5-0.42*(temperature-100);
			(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_bottom_=117.5-0.42*(temperature-100);		
}


//To compensate for the varying densities in the base units, we scale the total number of transistors in the design.
//Note: we enforce that the max transistor density NEVER exceeds
void sesctherm3Dmaterial::calc_die_transistor_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_){
	chip_flp_unit* source_chip_flp=(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].source_chip_flp_;
	double unit_width=(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].x2_;
	double unit_height=(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].z2_;
	double unit_thickness=(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].y2_;
	
	double chip_thickness=datalibrary_->all_layers_info_[layer]->thickness_;
	double chip_width=datalibrary_->all_layers_info_[layer]->width_;
	double chip_height=datalibrary_->all_layers_info_[layer]->height_;
	double transistor_width=datalibrary_->config_data_->sample_gate_width_;
	double silicon_film_length=datalibrary_->config_data_->sample_silicon_film_length_;
	double t_box=datalibrary_->config_data_->sample_box_thickness_;
	double k_si=datalibrary_->config_data_->sample_silicon_island_conductivity_;
	double p_ox_c_ox=datalibrary_->config_data_->sample_thermal_capacity_oxide_;
	double p_si_c_si=datalibrary_->config_data_->sample_thermal_capacity_silicon_;
	double num_transistors	= datalibrary_->config_data_->transistor_count_;
	int technology=datalibrary_->config_data_->technology_;			//get the technology used
    double pmos_transistor_size=sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_L_AVE][technology]*
													sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_PMOS_W_AVE][technology]; //p_mos area
	double nmos_transistor_size=sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_L_AVE][technology]*
													sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_NMOS_W_AVE][technology]; //n_mos area
																																		
	double t_sub=datalibrary_->config_data_->sample_silicon_substrate_thickness_;
	double t_si=datalibrary_->config_data_->sample_silicon_film_thickness_;
	double k_ox_eff=datalibrary_->config_data_->sample_oxide_conductivity_transistor_;
	double k_box=datalibrary_->config_data_->sample_oxide_conductivity_;
	
	double t_interconnect=0;
	int num_layers = (int)sesctherm3Ddatalibrary::technology_parameters_[TECH_LEVELS][technology];
	for(int i=0;i<num_layers;i++){
			t_interconnect = sesctherm3Ddatalibrary::technology_parameters_[10+(i*9)+TECH_H][technology]; //get the thickness of the layer for the given technology
	}
		
	

	
	double percentage_box=t_box/(t_sub+t_box+t_si+t_interconnect);	
	double box_thickness_actual=chip_thickness*percentage_box;
	double percentage_silicon_film=t_si/(t_sub+t_box+t_si+t_interconnect);	
	double silicon_film_thickness=chip_thickness*percentage_silicon_film;
	
	//we have to compute the density value used as the scaling factor
	//We cannot have more than N transistors on the chip, the density numbers determine how the
	//available transistors are distributed.
	//this is done as follows
	//SUM(i, 0, N)(flp_density_i*area_i*base_density)=Total number of transistors
	//then model unit density equals source_chip_flp->density*base_density (transistors/area^2)
	
	vector<chip_flp_unit>& flp_units=(*datalibrary_->all_layers_info_[layer]->chip_floorplan_).flp_units_;
	
	double total=0;
	for(int i=0;i<(int)flp_units.size();i++){
		total+=flp_units[i].area_*flp_units[i].chip_density_;
	}
	double base_density=num_transistors/total;		//this is the density used as a baseline, all the values are scaled from this
	
	

	
	//compute effective density (transistors/m^2)
	double transistor_density=source_chip_flp->chip_density_*base_density;
	double interconnect_density=source_chip_flp->interconnect_density_;

		

	
	
	num_transistors	= transistor_density*unit_width*unit_height;		//this is the number of transistors for the cross-sectional region

	
		//compute the conductivity from the chip to interconnect
	//R_top=interlayer_resistance (compute)
	double conduct_bottom=sesctherm3Dmaterial::calc_effective_interconnect_layer_vertical_conductivity(0, interconnect_density, datalibrary_);	//Layer 0, default density
	
	//STORE THE VERTICAL CONDUCTIVITY DOWNWARDS
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_bottom_=conduct_bottom;
	
	//compute the resistance from the transistor through the BOX and FOX to the silicon substrate
	//R_bottom=R_box=[t_box/(k_ox_eff*width)](L_si)
	double transistor_vertical_resistance_down=(box_thickness_actual/(k_ox_eff*transistor_width*silicon_film_length));
	
	//k_vertical=l/R_bottom*A=(t_si+t_box)/R_bottom*transistor_width*L_si;
	double transistor_conductivity_vertical=(silicon_film_thickness+box_thickness_actual)/(transistor_vertical_resistance_down*transistor_width*silicon_film_length);
	
	//R_lateral=L_si/[width*(k_ox_eff(t_box/3) + k_si*t_si)]
	double transistor_lateral_resistance=silicon_film_length/(transistor_width*(k_ox_eff*(box_thickness_actual/3) + k_si*silicon_film_thickness));
	
	//k_lateral=L_si/(R_lateral*transistor_width*(t_si+t_box))
	double transistor_conductivity_lateral=silicon_film_length/(transistor_lateral_resistance*transistor_width*(silicon_film_thickness+box_thickness_actual));
	
	//C_eff=(L_si)(width)[p_ox*c_ox*t_box/3+ p_si*c_si*t_si]=density*spec_heat*volume
	double effective_capacitance=silicon_film_length*transistor_width*(p_ox_c_ox*(box_thickness_actual/3) + p_si_c_si*silicon_film_thickness);
		

	
	//transistor density*transistor_specific_heat=C_eff/[(t_si + t_box/3)(L_si)(width)]

	double transistor_p_eff_c_eff=effective_capacitance/((silicon_film_thickness+box_thickness_actual/3)*(silicon_film_length)*(transistor_width));
	
	//COMPUTE EFFECTIVE LATERAL  conductivity given the transistor effective lateral/vertical conductivities
	
	//compute %silicon based upon the average size of the transistor and the number of transistors for the design
	//assume average sized transistors and 50% pullup/pulldown overall
	
	double transistor_area=.5*num_transistors*pmos_transistor_size+.5*num_transistors*nmos_transistor_size;
	
	//COMPUTE THE PERCENTAGE OF THE DIE THAT IS COVERED BY TRANSISTORS (should be roughly 70% for 130nm technology), the rest is poly
	double percent_transistor=transistor_area/(unit_height*unit_width);
	double percent_silicon=1-percent_transistor;
	
	//if we EVER use more than 100% of the total die area for transistors, we did something wrong
	if(percent_silicon<0){
		cerr << "sesctherm3Dmaterial::calc_die_transistor_layer_properties_dyn() => We are using more than 100% of the transistors with percent_transistor=[" << percent_transistor << "]" << endl;
		cerr << "Note: this model does NOT allow the granularity to be less than that of a single transistor" << endl;
		exit(1);
	}
	
	//given the percentage of the die that is silicon (and not transistors), compute the silicon lateral resistance
	//R_silicon=unit_width/(k_si*unit_height*si_thickness*%silicon)
	double tmp=datalibrary_->layer_materials_.find("BULK_SI").conductivity_;
	double silicon_effective_laterial_resistance=unit_width/(datalibrary_->layer_materials_.find("BULK_SI").conductivity_*unit_height*silicon_film_thickness*percent_silicon);
	
	//given the percentage of the die that is transistors (and not silicon), compute the silicon lateral resistance
	//R_transistor=unit_width/(k_transistor_lateral*unit_height*si_thickness*%transistor)
	double transistor_effective_lateral_resistance=unit_width/(transistor_conductivity_lateral*unit_height*silicon_film_thickness*percent_transistor);
		
	//R_box=unit_width/(k_box*unit_height*box_thickness)
	double box_resistance=unit_width/(k_box*unit_height*box_thickness_actual);
	
	//R_eq_lateral=1/ (1/R_silicon+1/R_transistor+1/R_box)
	double r_eq_lateral=1/ ((1/silicon_effective_laterial_resistance) + (1/transistor_effective_lateral_resistance) + (1/box_resistance));
	
	//k_eq_lateral=unit_width/(R_eq_lateral*layer_thickness*unit_height)
	double k_eq_lateral=unit_width/(r_eq_lateral*(silicon_film_thickness+box_thickness_actual)*unit_height);

						//STORE THE LATERAL CONDUCTIVITY
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_left_=k_eq_lateral;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_right_=k_eq_lateral;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_up_=k_eq_lateral;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_down_=k_eq_lateral;

//COMPUTE EFFECTIVE VERTICAL DOWN conductivity given the transistor effective lateral/vertical conductivities
	//R_transistor=l/(k_transistor_vertical*A)=layer_thickness/(k_transistor_vertical*width_unit*height_unit*%transistors)
	
	double r_transistor=unit_thickness/(transistor_conductivity_vertical*unit_width*unit_height*percent_transistor);

	//R_fox=l/(k_fox_box*A)=layer_thickness/(k_fox_box*height_unit*width_unit*%silicon)
	double r_fox=unit_thickness/(k_box*unit_height*unit_width*percent_silicon);
	
	//R_eq_vertical_down=1/( (1/R_transistor) + (1/R_fox) )
	double r_eq_vertical_down= 1/ ( (1/r_transistor) + (1/r_fox) );
	
	//k_eq_vertical_down=l/R_eq_vertical_down*A=layer_thickness/(R_eq_vertical_down*unit_height*unit_width)
	double k_eq_vertical_down= unit_thickness/(r_eq_vertical_down*unit_height*unit_width);

	
	//STORE THE VERTICAL CONDUCTIVITY UPWARDS
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_top_=k_eq_vertical_down;
	
	//COMPUTE EFFECTIVE density*spec_heat value
	//FIXME: this is a crude method of computing the density/specific heat
	//Density and Specific heat is scaled by density as follows:
	//We don't know the exact density and specific heat but we know the product of the two.
	//If we assume that the density and specific heat scales as a function of the area, then we can compute
	//a weighted average of the density*specific heat of the fox/box and the transistors/box 
	//%transistor/box*(density_transistor*specific_heat_transistor)+%fox/box*(density_fox*specific_heat_fox)
	double p_eff_c_eff=(percent_transistor*transistor_p_eff_c_eff + percent_silicon*p_ox_c_ox);

	//we don't know the exact density and specific heat, so what we store is p_eff_c_eff
	//this means that we set the specific heat to 1 and set the density to be equal to p_eff_c_eff, assuming average density
	
	//STORE THE LAYER p_eff_c_eff (or set spec_heat to 1 and density to density_eff * spec_heat_eff)

	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].specific_heat_=1;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].row_=p_eff_c_eff;
	
	//store the emissivity using a simple weighting rule
	double emissivity=(percent_transistor)*(percentage_box)*(datalibrary_->layer_materials_.find("SIMOX").emissivity_)+
								(percent_silicon)*(datalibrary_->layer_materials_.find("SIMOX").emissivity_)+
								(percent_transistor)*(percentage_silicon_film)*(datalibrary_->layer_materials_.find("DOPED_POLYSILICON").emissivity_);
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].emissivity_=emissivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].heat_transfer_methods_center_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);	
	
}


//FIXME: this is currently implemented using the following parameters
//This is for a simplified SOI Mosfet based a 250nm lithography process
//This should be scaled for ANY technology
//"Efficient Thermal Modeling of SOI MOSFETs for Fast Dynamic Operation"
//FIXME: Strained silicon should be added (SOI is only modeled right now)
//FIXME: Silicon conductivity should be a function of temperature
/*
Oxide Thickness				tox			3nm
Gate Length					L_g			0.22um
Fox Length					L_fox		2um
Eff Channel Length			L_ef		0.18um
Source/Drain Contact Length	L_cd,L_cd	0.25um
Silicon Substrate Thickness	t_sub		1.5um
Silicon Film Length			L_si		1.18um
Silicon Film Thickness		t_si		0.04um
Box thickness				t_BOX		0.15um
Vthreshold					Vth			0.22V
Oxide thermal conductivity	K_ox		0.82W/mK
Silicon Island thermal Conductivity	K_si	63W/mK
Effective Oxide thermal conductivity K_ox_eff 1.79W/mK (FIXME: this should be computed, currently extrapolated from above paper)
Thermal Capacity Density	p_si*c_si	1.63x10^6J/M^3*K
width=2.86um	(FIXME: this value should be updated)


R_top=interlayer_resistance (compute)
R_bottom=R_box=[t_box/(k_ox_eff*width)](L_si)
k_vertical=l/R_bottom*A=(t_si+t_box)/R_bottom*transistor_width*L_si;

R_lateral=1/[width*(k_ox_eff(t_box/3) + k_si*t_si)]
k_lateral=L_si/(R_lateral*transistor_width*(t_si+t_box))

C_eff=(L_si)(width)[p_ox*c_ox*t_box/3+ p_si*c_si*t_si]=density*spec_heat*volume

We want the volume to be that of the cross-sectional region, hence we divide by the volume
used here to obtain density*spec_heeat

spec_heat=1 //we don't know the exact specific heat, only the densith*spec_heat
transistor density=C_eff/[(t_si + t_box/3)(L_si)(width)]

We want the conductivities, not the resistances: obtain as follows


Note: this means that we WON'T have the exact specific heat and density for the die_transistor layer

We are assuming that the time periods here are significantly longer than the thermal time constant 

Once we obtain these value, we scale them based upon the density of the transistors. 

For the lateral resistances, we obtain a parallel resistance between the transistor resistance, silicon resistance and box/fox resistance:
R_silicon=chip_width/(k_si*chip_height*si_thickness*%silicon)
R_transistor=chip_width/(k_transistor_lateral*chip_height*si_thickness*%transistor)
R_box=chip_width/(k_box*chip_height*box_thickness)
R_eq_lateral=1/ (1/R_silicon+1/R_transistor+1/R_box)
k_eq_lateral=chip_width/(R_eq_lateral*layer_thickness*chip_height)

Note: we don't know the exact silicon film thickness and box thickness. All we know if the entire thickness of the box/fox layer.

For the top resistance/conductivity, this is computed via an external function which compensates for density

The botom resistance is scaled by computing the parallel resistance between the transistor resistance and box/fox resistance:
R_transistor=l/(k_transistor_vertical*A)=layer_thickness/(k_transistor_vertical*width_chip*height_chip*%transistors)
R_fox=l/(k_fox_box*A)=layer_thickness/(k_fox_box*height_chip*width_chip*%silicon)
R_eq_vertical_down=1/( (1/R_transistor) + (1/R_fox) )
k_eq_vertical_down=l/R_eq_vertical_down*A=layer_thickness/(R_eq_vertical_down*chip_height*chip_width)
FIXME: this is a crude method of computing the density/specific heat
Density and Specific heat is scaled by density as follows:
We don't know the exact density and specific heat but we know the product of the two.
If we assume that the density and specific heat scales as a function of the area, then we can compute
a weighted average of the density*specific heat of the fox/box and the transistors/box 
%transistor*(density_transistor*specific_heat_transistor)+%fox*(density_fox*specific_heat_fox)
*/
void sesctherm3Dmaterial::calc_die_transistor_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){

	double chip_thickness;
	if(layer.thickness_==0) chip_thickness=datalibrary_->config_data_->chip_thickness_;
	else chip_thickness=layer.thickness_;
	double chip_width;
	if(layer.width_==0) chip_width=datalibrary_->config_data_->chip_width_;
	else chip_width=layer.width_;
	double chip_height;
	if(layer.height_==0) chip_height=datalibrary_->config_data_->chip_height_;
	else chip_height=layer.height_;
	
	double transistor_width=datalibrary_->config_data_->sample_gate_width_;
	double silicon_film_length=datalibrary_->config_data_->sample_silicon_film_length_;
	double t_box=datalibrary_->config_data_->sample_box_thickness_;
	double k_si=datalibrary_->config_data_->sample_silicon_island_conductivity_;
	double p_ox_c_ox=datalibrary_->config_data_->sample_thermal_capacity_oxide_;
	double p_si_c_si=datalibrary_->config_data_->sample_thermal_capacity_silicon_;
	double num_transistors	= datalibrary_->config_data_->transistor_count_;
	int technology=datalibrary_->config_data_->technology_;			//get the technology used
    double pmos_transistor_size=sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_L_AVE][technology]*
													sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_PMOS_W_AVE][technology]; //p_mos area
	double nmos_transistor_size=sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_L_AVE][technology]*
													sesctherm3Ddatalibrary::technology_parameters_[TECH_GATE_NMOS_W_AVE][technology]; //n_mos area
																					

	
//first we need to find the actual thickness of the buried oxide and silicon film thickness
//we assume that the box/silicon film thickness is in the same proportions for the sample transistor 
//as it is in the target technology 

//FIXME: we should actually be obtaining approximate values that vary with process technology and foundry (IBM/TSMC/INTEL)

	double t_sub=datalibrary_->config_data_->sample_silicon_substrate_thickness_;
	double t_si=datalibrary_->config_data_->sample_silicon_film_thickness_;
	double k_ox_eff=datalibrary_->config_data_->sample_oxide_conductivity_transistor_;
	double k_box=datalibrary_->config_data_->sample_oxide_conductivity_;
	
	double t_interconnect=0;
	int num_layers = (int)sesctherm3Ddatalibrary::technology_parameters_[TECH_LEVELS][technology];
	for(int i=0;i<num_layers;i++){
	  t_interconnect = sesctherm3Ddatalibrary::technology_parameters_[10+(i*9)+TECH_H][technology]; //get the thickness of the layer for the given technology
	}
			
	double percentage_box=t_box/(t_sub+t_box+t_si+t_interconnect);	
	double box_thickness_actual=chip_thickness*percentage_box;
	double percentage_silicon_film=t_si/(t_sub+t_box+t_si+t_interconnect);	
	double silicon_film_thickness=chip_thickness*percentage_silicon_film;
	
	
	if(layer.width_==0)layer.width_=chip_width;
	if(layer.height_==0)layer.height_=chip_height;
	if(layer.thickness_==0)layer.thickness_=box_thickness_actual+silicon_film_thickness;
	
	
	//compute the conductivity from the chip to interconnect
	//R_bottom=interlayer_resistance (compute)
	double conduct_bottom=sesctherm3Dmaterial::calc_effective_interconnect_layer_vertical_conductivity(0, 1.0, datalibrary_);	//Layer 0, default density
	
	//STORE THE VERTICAL CONDUCTIVITY DOWNWARDS
	layer.vertical_conductivity_down_=conduct_bottom;
	
	//compute the resistance from the transistor through the BOX and FOX to the silicon substrate
	//R_bottom=R_box=[t_box/(k_ox_eff*width)](L_si)
	double transistor_vertical_resistance_down=(box_thickness_actual/(k_ox_eff*transistor_width*silicon_film_length));
	
	//k_vertical=l/R_bottom*A=(t_si+t_box)/R_bottom*transistor_width*L_si;
	double transistor_conductivity_vertical=(silicon_film_thickness+box_thickness_actual)/(transistor_vertical_resistance_down*transistor_width*silicon_film_length);
	
	//R_lateral=L_si/[width*(k_ox_eff(t_box/3) + k_si*t_si)]
	double transistor_lateral_resistance=silicon_film_length/(transistor_width*(k_ox_eff*(box_thickness_actual/3) + k_si*silicon_film_thickness));
	
	//k_lateral=L_si/(R_lateral*transistor_width*(t_si+t_box))
	double transistor_conductivity_lateral=silicon_film_length/(transistor_lateral_resistance*transistor_width*(silicon_film_thickness+box_thickness_actual));
	
	//C_eff=(L_si)(width)[p_ox*c_ox*t_box/3+ p_si*c_si*t_si]=density*spec_heat*volume
	double effective_capacitance=silicon_film_length*transistor_width*(p_ox_c_ox*(box_thickness_actual/3) + p_si_c_si*silicon_film_thickness);
		

	
	//transistor density*transistor_specific_heat=C_eff/[(t_si + t_box/3)(L_si)(width)]=C_eff/volume

	double transistor_p_eff_c_eff=effective_capacitance/((silicon_film_thickness+box_thickness_actual/3)*(silicon_film_length)*(transistor_width));
	
	//COMPUTE EFFECTIVE LATERAL  conductivity given the transistor effective lateral/vertical conductivities
	
	//compute %silicon based upon the average size of the transistor and the number of transistors for the design
	//assume average sized transistors and 50% pullup/pulldown overall
	
	double transistor_area=.5*num_transistors*pmos_transistor_size+.5*num_transistors*nmos_transistor_size;
	
	//COMPUTE THE PERCENTAGE OF THE DIE THAT IS COVERED BY TRANSISTORS (should be roughly 70% for 130nm technology), the rest is poly
	double percent_transistor=transistor_area/(chip_height*chip_width);
	double percent_silicon=1-percent_transistor;
	
	//if we EVER use more than 100% of the total die area for transistors, we did something wrong
	if(percent_silicon<0){
		cerr << "sesctherm3Dmaterial::calc_die_transistor_layer_properties() => We are using more than 100% of the transistors with percent_transistor=[" << percent_transistor << "]" << endl;
		exit(1);
	}
	
	//given the percentage of the die that is silicon (and not transistors), compute the silicon lateral resistance
	//R_silicon=chip_width/(k_si*chip_height*si_thickness*%silicon)
	double tmp=datalibrary_->layer_materials_.find("BULK_SI").conductivity_;
	double silicon_effective_laterial_resistance=chip_width/(datalibrary_->layer_materials_.find("BULK_SI").conductivity_*chip_height*silicon_film_thickness*percent_silicon);
	
	//given the percentage of the die that is transistors (and not silicon), compute the silicon lateral resistance
	//R_transistor=chip_width/(k_transistor_lateral*chip_height*si_thickness*%transistor)
	double transistor_effective_lateral_resistance=chip_width/(transistor_conductivity_lateral*chip_height*silicon_film_thickness*percent_transistor);
		
	//R_box=chip_width/(k_box*chip_height*box_thickness)
	double box_resistance=chip_width/(k_box*chip_height*box_thickness_actual);
	
	//R_eq_lateral=1/ (1/R_silicon+1/R_transistor+1/R_box)
	double r_eq_lateral=1/ ((1/silicon_effective_laterial_resistance) + (1/transistor_effective_lateral_resistance) + (1/box_resistance));
	
	//k_eq_lateral=chip_width/(R_eq_lateral*layer_thickness*chip_height)
	double k_eq_lateral=chip_width/(r_eq_lateral*(silicon_film_thickness+box_thickness_actual)*chip_height);
	
	//STORE THE LATERAL CONDUCTIVITY
	layer.horizontal_conductivity_=k_eq_lateral;
	
	//COMPUTE EFFECTIVE VERTICAL DOWN conductivity given the transistor effective lateral/vertical conductivities
	//R_transistor=l/(k_transistor_vertical*A)=layer_thickness/(k_transistor_vertical*width_chip*height_chip*%transistors)
	
	double r_transistor=layer.thickness_/(transistor_conductivity_vertical*chip_width*chip_height*percent_transistor);

	//R_fox=l/(k_fox_box*A)=layer_thickness/(k_fox_box*height_chip*width_chip*%silicon)
	double r_fox=layer.thickness_/(k_box*chip_height*chip_width*percent_silicon);
	
	//R_eq_vertical_down=1/( (1/R_transistor) + (1/R_fox) )
	double r_eq_vertical_down= 1/ ( (1/r_transistor) + (1/r_fox) );
	
	//k_eq_vertical_down=l/R_eq_vertical_down*A=layer_thickness/(R_eq_vertical_down*chip_height*chip_width)
	double k_eq_vertical_down= layer.thickness_/(r_eq_vertical_down*chip_height*chip_width);
	
	//STORE THE VERTICAL CONDUCTIVITY UPWARDS
	layer.vertical_conductivity_up_=k_eq_vertical_down;
	
	//COMPUTE EFFECTIVE density*spec_heat value
	//FIXME: this is a crude method of computing the density/specific heat
	//Density and Specific heat is scaled by density as follows:
	//We don't know the exact density and specific heat but we know the product of the two.
	//If we assume that the density and specific heat scales as a function of the area, then we can compute
	//a weighted average of the density*specific heat of the fox/box and the transistors/box 
	//%transistor/box*(density_transistor*specific_heat_transistor)+%fox/box*(density_fox*specific_heat_fox)
	double p_eff_c_eff=(percent_transistor*transistor_p_eff_c_eff + percent_silicon*p_ox_c_ox);

	//we don't know the exact density and specific heat, so what we store is p_eff_c_eff
	//this means that we set the specific heat to 1 and set the density to be equal to p_eff_c_eff, assuming average density
	
	//STORE THE LAYER p_eff_c_eff (or set spec_heat to 1 and density to density_eff * spec_heat_eff)
	layer.spec_heat_=1;
	layer.density_=p_eff_c_eff;
	
	//store the emissivity using a simple weighting rule between BOX/FOX/and POLY
	double emissivity=(percent_transistor)*(percentage_box)*(datalibrary_->layer_materials_.find("SIMOX").emissivity_)+
								(percent_silicon)*(datalibrary_->layer_materials_.find("SIMOX").emissivity_)+
								(percent_transistor)*(percentage_silicon_film)*(datalibrary_->layer_materials_.find("DOPED_POLYSILICON").emissivity_);
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);	
}



//this is used to compute the relationship between k_ild and K_ild
//where k_ild is the dielectric contact (process specific) and K_ild is the inter-layer conductivity

//
//
//FIXME: material 0 is the only one currently implemented (get the other parameters)
//
//
// parameters (defaults to xerogel)
//	Material 0: xerogel (porous silicate)

//			 R. M. Costescu, A. J. Bullen, G. Matamis, K. E. O’Hara, and D. G. 
//			Cahill, “Thermal conductivity and sound velocities of hydrogen- 
//			silsesquioxane low-k dielectrics,” Phys. Rev. B, vol. 65, no. 9, pp. 
//			094205/1-6, 2002.

//			A. Jain, S. Rogojevic, S. Ponoth, W. N. Gill, J. L. Plawsky, E. 
//			Simonyi, S. T. Chen, and P. S. Ho, “Processing dependent thermal 
//			conductivity of nanoporous silica xerogel films,” J. Appl. Phys., vol. 
//			91, no. 5, pp. 3275-3281, 2002. 

//			B. Y. Tsui, C. C. Yang, and K. L. Fang, “Anisotropic thermal 
//			conductivity of nanoporous silica film,” IEEE Trans. Electron 
//			Devices, vol. 51, no. 1, pp. 20-27, Jan. 2004. 

//	Material 1: flourinated silicate glass (FSG) 

//			B. C. Daly, H. J. Maris, W. K. Ford, G. A. Antonelli, L. Wong, and 
//			E. Andideh, “Optical pump and probe measurement of the thermal 
//			conductivity of low-k dielectno. 10, pp. 6005-6009, 2002. 

//	Material 2: hydrogen-sisesquioxane (HSQ)

//			R. M. Costescu, A. J. Bullen, G. Matamis, K. E. O’Hara, and D. G. 
//			Cahill, “Thermal conductivity and sound velocities of hydrogen- 
//			silsesquioxane low-k dielectrics,” Phys. Rev. B, vol. 65, no. 9, pp. 
//			094205/1-6, 2002. 

//			P. Zhou, Ph. D. Dissertation, Stanford University, 2001. 

//	Material 3: carbon-doped oxide (CDO)

//			B. C. Daly, H. J. Maris, W. K. Ford, G. A. Antonelli, L. Wong, and 
//			E. Andideh, “Optical pump and probe measurement of the thermal 
//			conductivity of low-k dielectric thin films,”  J. Appl. Phys., vol. 92, 
//			no. 10, pp. 6005-6009, 2002. 

//			P. Zhou, Ph. D. Dissertation, Stanford University, 2001. 

//	Material 4: organic polymers

//			P. Zhou, Ph. D. Dissertation, Stanford University, 2001. 

//	Material 5: methylsilsesquioxane (MSQ)

//			J. Liu, D. Gan, C. Hu, M. Kiene, P. S. Ho, W. Volksen, and R. D. 
//			Miller, “Porosity effect on the dielectric constant and 
//			thermomechanical properties of organosilicate films,”  Appl. Phys. 
//			Lett., vol. 81, no. 22, pp. 4180-4182, 2002. 

//		Material 6: phosphosilicate glass (PSG)
//			this is assumed to be the pre-metal inter-layer dielectric
//			FIXME: we assume a thermal conductivity of 0.94W/mk ALWAYS!
//

//	k_p=dielectric constant of pores
//	k_m=dielectric constant of matrix material 
//	K_p=cond_p=thermal conductivity of pores 
//	K_m=cond_m=thermal conductivity of matrix material
//	x is a fitting parameter

 double sesctherm3Dmaterial::calc_interlayer_conductivity(double dielectric_constant, int32_t material, sesctherm3Ddatalibrary* datalibrary_){
	double k_p, k_m, cond_p, cond_m, x;
	switch(material){
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			k_p=1.0;
			k_m=4.1;
			cond_p=0.0255;
			cond_m=1.4;
			x=0.49;
			break;
		case 6:
			return(0.94);		//FIXME: PSG is assumed to have thermal conductivity of 0.94W/mK always!
		default:
			cerr << "sesctherm3Dmaterial::calc_interlayer_conductivity -> invalid material (assuming material 0, xerogel)" << endl;
			k_p=1.0;
			k_m=4.1;
			cond_p=0.0255;
			cond_m=1.4;
			x=0.49;
	}
	
	map<double,map<int,double> >::iterator iter = datalibrary_->config_data_->inter_layer_conductivities_.find(dielectric_constant);
	if( iter != datalibrary_->config_data_->inter_layer_conductivities_.end() ) {
		map<int,double>::iterator iter2 = iter->second.find(material);
		if( iter2 != iter->second.end() ) {
			return iter2->second;			//return the cached value if it exists
		}
	}
	//otherwise compute the value
	
	RegressionLine::Points	k_ild_to_P;
	RegressionLine::Points   P_to_cond_ild;
	
	for(double p=0;LT(p,1.0);p+=.001){
		//k_ild_to_P[(2*k_m + 3*p*k_p - 3*p*k_m)/4]=p;	//ignore imaginary portion
		double tmp=(p*cond_p + (1-p)*cond_m) * (1-pow(p,x)) + ( (cond_p*cond_m*powf(p,x)) / (p*cond_m + (1-p)*cond_p) );
		P_to_cond_ild[p]= tmp;
	}
	
//	RegressionLine k_ild_to_P_line(k_ild_to_P);
	
    //normalize inter_layer_dielectric to three digits
	//perform polynomial interpolation of k_ild_to_P and P_to_cond_ild
	//find P based upon k_ild (ignore imaginary portion)
	//then find cond_ild based upon P
	double tmp0=floor(dielectric_constant * powf(10.0, 3.0) + .5);
	double tmp1= powf(10.0, 3.0);
	double k_ild=tmp0/tmp1;
	double p_lookup=(k_ild-k_m)*(k_p+2*k_ild)/(-4*powf(k_ild,2.0)+3*k_ild*(k_p-k_m));
	double value=RegressionLine::quad_interpolate(P_to_cond_ild, p_lookup);
	datalibrary_->config_data_->inter_layer_conductivities_[dielectric_constant][material]=value;
	return(RegressionLine::quad_interpolate(P_to_cond_ild, p_lookup));
}

//density is the percentage density of the interconnect relative to the average density (50% of SRAM)
//this is used to scale the effective conductivity as a function of the density of the corresponding functional unit
 double sesctherm3Dmaterial::calc_effective_interconnect_layer_vertical_conductivity(int layer, double density, sesctherm3Ddatalibrary* datalibrary_){
    int32_t technology=datalibrary_->config_data_->technology_;			//get the technology used
	int metal_index=sesctherm3Ddatalibrary::tech_metal_index(layer);	//get the metal layer offset for the technology_parameters table
    double dielectric_constant=sesctherm3Ddatalibrary::technology_parameters_[TECH_K][technology];
	//compute the bulk thermal conductivity of the inter-layer dielectric using the dielectric constant
	
    double bulk_thermal_conductivity;
	
	if(layer==0)
		bulk_thermal_conductivity=calc_interlayer_conductivity(dielectric_constant, 6, datalibrary_); //PMD (pre-metal dielectric is PSG)
	else
		bulk_thermal_conductivity=calc_interlayer_conductivity(dielectric_constant, 0,  datalibrary_); //PMD (pre-metal dielectric is PSG)
	
	double metal_width=sesctherm3Ddatalibrary::technology_parameters_[metal_index+TECH_W][technology];
	double metal_spacing=sesctherm3Ddatalibrary::technology_parameters_[metal_index+TECH_SPACE][technology];
	double via_size=metal_width; //this assumes that the via size is the same as the metal width;
	double inter_via_distance=sesctherm3Ddatalibrary::technology_parameters_[metal_index+TECH_VIADIST][technology]*3*(1/density);
	double via_thermal_conductivity;
	if(layer==0)
		via_thermal_conductivity=165;	//tungsten has thermal conductivity of 165W/mK
	else
		via_thermal_conductivity=396.36; //copper thermal conductivityis 396.36W/mK at 85 degrees celcius.
	
	double via_density=(via_size*via_size)/ ( (metal_width+metal_spacing)*inter_via_distance );
	double cond_eff=via_density*via_thermal_conductivity+ (1-via_density)*bulk_thermal_conductivity;
	return(cond_eff);
}

//this computes the horizontal conductivity across the layer
 double sesctherm3Dmaterial::calc_effective_interconnect_layer_horizontal_conductivity(int material_layer, int32_t interconnect_layer, double density, sesctherm3Ddatalibrary* datalibrary_){
	 double height;
	 if(datalibrary_->all_layers_info_[material_layer]->height_==0)height=datalibrary_->config_data_->chip_height_;
	 else height=datalibrary_->all_layers_info_[material_layer]->height_;
	 double width;
	 if(datalibrary_->all_layers_info_[material_layer]->width_==0)width=datalibrary_->config_data_->chip_width_;
	 else width=datalibrary_->all_layers_info_[material_layer]->width_;
	 
		
    int32_t technology=datalibrary_->config_data_->technology_;			//get the technology used
    double dielectric_constant=sesctherm3Ddatalibrary::technology_parameters_[TECH_K][technology];
	//compute inter-layer dielectric bulk thermal conductivity
    double bulk_thermal_conductivity;
	if(interconnect_layer==0)
		bulk_thermal_conductivity=calc_interlayer_conductivity(dielectric_constant,6, datalibrary_); //PMD (pre-metal dielectric is PSG)
	else
		bulk_thermal_conductivity=calc_interlayer_conductivity(dielectric_constant,0, datalibrary_); //PMD (pre-metal dielectric is PSG)
	
	double layer_area=0;
	double metal_area=0;
	//compute metal layer volume
	layer_area=height*width;
	if(interconnect_layer==0)
		metal_area=layer_area*min(.71*density, 1.0);
	else
		metal_area=layer_area*min(.60*density, 1.0);

	double copper_conductivity = datalibrary_->layer_materials_.find("COPPER").conductivity_;
	double total_wire_crosssectional_area=(metal_area/width)*sesctherm3Ddatalibrary::technology_parameters_[10+interconnect_layer*9+TECH_H][technology];
	double total_dielectric_crossectional_area=((layer_area-metal_area)/width)*sesctherm3Ddatalibrary::technology_parameters_[10+interconnect_layer*9+TECH_H][technology];
	double total_resistance=1/( 1/(width/(copper_conductivity*total_wire_crosssectional_area)) + 1/(width/(bulk_thermal_conductivity*total_dielectric_crossectional_area)));
	double total_conductivity=width/(total_resistance*height*sesctherm3Ddatalibrary::technology_parameters_[10+interconnect_layer*9+TECH_H][technology]);
	return(total_conductivity);
}



void sesctherm3Dmaterial::calc_interconnect_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_){
	chip_flp_unit* source_chip_flp=(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].source_chip_flp_;
	int technology=datalibrary_->config_data_->technology_;			//get the technology used
	double height;
 	if(datalibrary_->all_layers_info_[layer]->height_==0)
		height=datalibrary_->config_data_->chip_height_;
	else 
		height=datalibrary_->all_layers_info_[layer]->height_;
	double width;
	if(datalibrary_->all_layers_info_[layer]->width_==0)
		width=datalibrary_->config_data_->chip_width_;
	else 
		width=datalibrary_->all_layers_info_[layer]->width_;
	double thickness;
 	if(datalibrary_->all_layers_info_[layer]->thickness_==0)
		thickness=datalibrary_->config_data_->chip_thickness_;
	else 
		thickness=datalibrary_->all_layers_info_[layer]->thickness_;
	
	int num_layers = (int)sesctherm3Ddatalibrary::technology_parameters_[TECH_LEVELS][technology];
	
	double interconnect_density=source_chip_flp->interconnect_density_;
		
	//compute the effective vertical thermal conductivity for all of the metal layers

	double area=0;
	double conductivity=0;
	double resistance=0;
	
	double temp_thickness=0;
	
	//here we compute the layer thickness for the sample technology parameters
	double total_thickness =0;
	for(int i=0;i<num_layers;i++){
		total_thickness += sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]; //get the thickness of the layer for the given technology
	}
	
	
	//here we assume the same propotions for the layers
	area = width * height;
	for(int i=0;i<num_layers;i++){
		temp_thickness = (sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness; //get the thickness of the layer for the given technology
		conductivity=sesctherm3Dmaterial::calc_effective_interconnect_layer_vertical_conductivity(i, interconnect_density, datalibrary_); //assume average conductivity initial computation 
		resistance += temp_thickness/(conductivity*area);
	}
	

		
	//k=l/AR
	double effective_vertical_conductivity = thickness/(area*resistance);
	
	//store the vertical conductivity
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_top_=effective_vertical_conductivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_bottom_=effective_vertical_conductivity;

	
	double conductance=0;

	//compute the effective horizontal thermal conductivity for all of the metal layers
	for(int i=0;i<num_layers;i++){
		temp_thickness = (sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness; //get the thickness of the layer for the given technology
		conductance+=sesctherm3Dmaterial::calc_effective_interconnect_layer_horizontal_conductivity(layer, i, interconnect_density, datalibrary_)*temp_thickness*height/width; //assume average conductivity initial computation 
	}
	resistance=1/conductance;	//compute effective resistance
	
	//k=l/AR
	double effective_horizontal_conductivity = width/(thickness*height*resistance);
	
	//store the horizontal conductivity
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_left_=effective_horizontal_conductivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_right_=effective_horizontal_conductivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_up_=effective_horizontal_conductivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].conduct_center_down_=effective_horizontal_conductivity;
	
	
	//calculate the effective specific heat for all of the metal layers
	//based upon data analysis, 71% wiring density of layer 1 and 60% on the remaining layers
	double layer_volume=0;
	double total_layer_volume=0;
	double total_metal_volume=0;
	//compute total metal layer volume
	for (int i=0;i<num_layers;i++){
		layer_volume=height*width*(sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness;
		total_layer_volume+=layer_volume;
		if(i==0)
			total_metal_volume+=layer_volume*min(.71*interconnect_density, 1.0);
		else
			total_metal_volume+=layer_volume*min(.60*interconnect_density,1.0);
		
	}
	
	double specific_heat=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").spec_heat_)+
								(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").spec_heat_);
	//store the specific heat
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].specific_heat_=specific_heat;
	
	
	//compute effective density
	double density=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").density_)+
								(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").density_);
	//store the density
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].row_=density;
	
	//store the emissivity
	double emissivity=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").emissivity_)+
								(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").emissivity_);
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].emissivity_=emissivity;
	(*datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_)[x][y].heat_transfer_methods_center_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
}


void sesctherm3Dmaterial::calc_interconnect_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	int technology = datalibrary_->config_data_->technology_;
	int num_layers = (int)sesctherm3Ddatalibrary::technology_parameters_[TECH_LEVELS][technology];
	
	double height;
 	if(layer.height_==0) 
		height=datalibrary_->config_data_->chip_height_;
	else 
		height=layer.height_;
		
	double width;
	if(layer.width_==0) 
		width=datalibrary_->config_data_->chip_width_;
	else 
		width=layer.width_;
		
	double thickness;
 	if(layer.thickness_==0) 
		thickness=datalibrary_->config_data_->chip_thickness_;
	else 
		thickness=layer.thickness_;
	
	double interconnect_density=1;

	//store the height
	if(layer.height_==0)layer.height_=height;
	
	//store the width
	if(layer.width_==0)layer.width_=width;
	
	//here we compute the layer thickness for the sample technology parameters
	double total_thickness =0;
	for(int i=0;i<num_layers;i++){
		total_thickness += sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]; //get the thickness of the layer for the given technology
	}
	
	
	//here we assume the same propotions for the layers
	double area = width * height;
	double conductivity=0;
	double resistance=0;
	double temp_thickness=0;
	for(int i=0;i<num_layers;i++){
		temp_thickness = (sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness; //get the thickness of the layer for the given technology
		conductivity=sesctherm3Dmaterial::calc_effective_interconnect_layer_vertical_conductivity(i, interconnect_density, datalibrary_); //assume average conductivity initial computation 
		resistance += temp_thickness/(conductivity*area);
	}
	
	//store the layer thickness
	if(layer.thickness_==0)layer.thickness_=total_thickness;
	
	//k=l/AR
	double effective_vertical_conductivity = total_thickness/(area*resistance);
	
	//store the vertical conductivity
	layer.vertical_conductivity_down_=effective_vertical_conductivity;
	layer.vertical_conductivity_up_=effective_vertical_conductivity;
	
	conductivity=0;
	double conductance=0;
	//compute the effective horizontal thermal conductivity for all of the metal layers
	for(int i=0;i<num_layers;i++){
		temp_thickness = (sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness; //get the thickness of the layer for the given technology
		conductance+=sesctherm3Dmaterial::calc_effective_interconnect_layer_horizontal_conductivity(layer.layer_number_, i, interconnect_density, datalibrary_)*temp_thickness*height/width; //assume average conductivity initial computation 
	}
	resistance=1/conductance;
	//k=l/AR
	double effective_horizontal_conductivity = width/(resistance*thickness*height);
	
	
	
	//store the horizontal conductivbity
	layer.horizontal_conductivity_=effective_horizontal_conductivity;


	//calculate the effective specific heat for all of the metal layers
	//based upon data analysis, 71% wiring density of layer 1 and 60% on the remaining layers
	double layer_volume=0;
	double total_layer_volume=0;
	double total_metal_volume=0;
	//compute total metal layer volume
	for (int i=0;i<num_layers;i++){
		layer_volume=height*width*(sesctherm3Ddatalibrary::technology_parameters_[10+i*9+TECH_H][technology]/total_thickness)*thickness;
		total_layer_volume+=layer_volume;
		if(i==0)
			total_metal_volume+=layer_volume*min(.71*interconnect_density, 1.0);
		else
			total_metal_volume+=layer_volume*min(.60*interconnect_density, 1.0);

	}

		double specific_heat=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").spec_heat_)+
			(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").spec_heat_);

	//store the specific heat
	layer.spec_heat_=specific_heat;
	
	
	//compute effective density
	double density=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").density_)+
								(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").density_);
	//store the density
	layer.density_=density;
	
	//store the emissivity
	double emissivity=(total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("COPPER").emissivity_)+
								(1-total_metal_volume/total_layer_volume)*(datalibrary_->layer_materials_.find("XEROGEL").emissivity_);
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
}



//CALCULATE HEAT SINK LAYER PARAMETERS
 void sesctherm3Dmaterial::calc_heat_sink_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double width=datalibrary_->config_data_->heat_sink_width_;
	double height=datalibrary_->config_data_->heat_sink_height_;
	double thickness=datalibrary_->config_data_->heat_sink_thickness_;
	double emissivity=datalibrary_->layer_materials_.find("COPPER").emissivity_;
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	if(layer.thickness_==0)layer.thickness_=thickness;
	layer.spec_heat_=datalibrary_->layer_materials_.find("COPPER").spec_heat_;
	layer.density_=datalibrary_->layer_materials_.find("COPPER").density_;
	layer.horizontal_conductivity_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_up_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_down_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
}

//CALCULATE HEAT SINK FINS LAYER PARAMETERS
 void sesctherm3Dmaterial::calc_heat_sink_fins_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double width=datalibrary_->config_data_->heat_sink_width_;
	double height=datalibrary_->config_data_->heat_sink_height_;
	double thickness=datalibrary_->config_data_->heat_sink_fins_thickness_;
	double emissivity=datalibrary_->layer_materials_.find("COPPER").emissivity_;
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	if(layer.thickness_==0)layer.thickness_=thickness;
	layer.spec_heat_=datalibrary_->layer_materials_.find("COPPER").spec_heat_;
	layer.density_=datalibrary_->layer_materials_.find("COPPER").density_;
	layer.horizontal_conductivity_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_up_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_down_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
}


//CALCULATE HEAT SPREADER LAYER PARAMETERS
 void sesctherm3Dmaterial::calc_heat_spreader_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double width=datalibrary_->config_data_->heat_spreader_width_;
	double height=datalibrary_->config_data_->heat_spreader_height_;
	double thickness=datalibrary_->config_data_->heat_spreader_thickness_;
	double emissivity=datalibrary_->layer_materials_.find("COPPER").emissivity_;
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	if(layer.thickness_==0)layer.thickness_=thickness;
	layer.spec_heat_=datalibrary_->layer_materials_.find("COPPER").spec_heat_;
	layer.density_=datalibrary_->layer_materials_.find("COPPER").density_;
	layer.horizontal_conductivity_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_down_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	layer.vertical_conductivity_up_=datalibrary_->layer_materials_.find("COPPER").conductivity_;
}

//CALCULATE C4_UNDERFILL LAYER PARAMETERS
 void sesctherm3Dmaterial::calc_c4underfill_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double width=datalibrary_->config_data_->chip_width_;
	double height=datalibrary_->config_data_->chip_height_;
	double thickness=90.55e-6; //90.55um (Takem from "Prediction of Thermal Performance...")
	double pin_count=datalibrary_->config_data_->pin_count_;
	double emissivity=datalibrary_->layer_materials_.find("COPPER").emissivity_;
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	if(layer.thickness_==0)layer.thickness_=thickness;
	double volume=width*height*thickness;
	layer.horizontal_conductivity_=0.8;	//0.8W/mK (Takem from "Prediction of Thermal Performance...")
	
	layer.vertical_conductivity_down_=1.66;	//1.66W/mK (Takem from "Prediction of Thermal Performance...")
	layer.vertical_conductivity_up_=1.66;	//1.66W/mK (Takem from "Prediction of Thermal Performance...")
	
	double volume_bump = (4/3)*3.14159265*(powf((250e-6)/2.0,3.0));
	layer.spec_heat_=datalibrary_->layer_materials_.find("COPPER").spec_heat_* ((volume_bump*pin_count)/volume);
	layer.density_=datalibrary_->layer_materials_.find("COPPER").density_* ((volume_bump*pin_count)/volume);
}



//CALCULATE PWB LAYER PARAMETERS
 void sesctherm3Dmaterial::calc_pwb_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	 double width;
	 if(layer.width_==0)width=datalibrary_->config_data_->package_width_;
	 else
		 width = layer.width_;
	 double height;
	 if(layer.height_==0)height=datalibrary_->config_data_->package_height_;
	 else 
		 height=layer.height_;
	 double thickness;
	 if(layer.thickness_==0)thickness=datalibrary_->config_data_->pwb_layer_total_thickness_;
	 else
		 thickness=layer.thickness_;
	double ave_side_length=(width+height)/2;
	double emissivity=0.8;	//PCB resin material
	
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
	//store the layer height and width
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	
	//First we need to determine the thickness of just the pwb itself. 
	//We only know the ENTIRE package thickness (not the individual portions)
	double total_thickness=0;
	//compute the total thickness of the sample stackup
	for(int i=0;i<(int)datalibrary_->config_data_->pwb_layer_thickness_.size();i++)
		total_thickness+=datalibrary_->config_data_->pwb_layer_thickness_[i];
		
	//store the thickness of the pwb from the sample stackup
	double pwb_layer_total_thickness=total_thickness;
	
	//finish accumulating the total thickness of the substrate+PCB regions
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++)
		total_thickness+=datalibrary_->config_data_->fcpbga_layer_thickness_[i];
	
	double percent=pwb_layer_total_thickness/total_thickness;		//this is the percent of the stackup that's the substrate layer
	
	//Calculate the actual thickness of the substrate layer by assuming that is the same proportions as the sample stackup
	
	//store the layer thickness
	if(layer.thickness_==0)layer.thickness_=thickness*percent;								//this is the actual thickness of the substrate
	
	thickness=layer.thickness_;
	
	double volume=width*height*layer.thickness_;
		
	//scale the layer thicknesses based upon the relative thickness of the pwb actually used
	vector<double> layer_thickness;
	
	for(int i=0;i<(int)datalibrary_->config_data_->pwb_layer_thickness_.size();i++){
		//this is the percentage of the substrate that this layer occupies
		double percentage=datalibrary_->config_data_->pwb_layer_thickness_[i]/pwb_layer_total_thickness;
		layer_thickness.push_back(layer.thickness_*percentage);	//determine the actual layer thickness
	}
	
	//calculates the equivalent specific heat for the entire layer 
	//this is SUM(i=1 to n, %volume_i * specific_heat_i)	
	double specific_heat=0;
	for(int i=0;i<(int)layer_thickness.size();i++){
		specific_heat += ((width*height*layer_thickness[i])/(width*height*layer.thickness_) * datalibrary_->config_data_->pwb_layer_specific_heat_[i]);
	}
	layer.spec_heat_=specific_heat;

	//calculates the equivalent  density for the entire layer 
	//this is SUM(i=1 to n, %volume_i * density_i)	
	double density=0;
	for(int i=0;i<(int)layer_thickness.size();i++){
		density += ((width*height*layer_thickness[i])/(width*height*layer.thickness_) * datalibrary_->config_data_->pwb_layer_density_[i]);
	}
	layer.density_=density;

	//this calculates the equivalent resistance for the entire layer (vertical)
	double resistance=0;
	for(int i=0;i<(int)layer_thickness.size();i++){
		resistance+=(layer_thickness[i]/
					( datalibrary_->config_data_->pwb_layer_conductivity_vert_[i]*width*height));
	}
	
    double conductivity_vertical = (layer.thickness_ / (width*height*resistance) );
    layer.vertical_conductivity_down_=conductivity_vertical;
	layer.vertical_conductivity_up_=conductivity_vertical;

	double conductivity=0;
	//this calculates the equivalent resistance for the entire layer (horizontal)
	for(int i=0;i<(int)layer_thickness.size();i++){
		conductivity+= datalibrary_->config_data_->pwb_layer_conductivity_horiz_[i];
		
	}
	conductivity/=((int)layer_thickness.size());	//find the average lateral resistance
    double conductivity_horizontal = conductivity;
	layer.horizontal_conductivity_=conductivity_horizontal;
}



//CALCULATE PACKAGE SUBTRATE PROPERTIES
 void sesctherm3Dmaterial::calc_package_substrate_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double width=datalibrary_->config_data_->chip_width_;
	double height=datalibrary_->config_data_->chip_height_;
	double ave_side_length=(width+height)/2;
	double thickness=datalibrary_->config_data_->package_thickness_;	//this is NOT the actual thickness (it includes a number of layers)
	double specific_heat=0;
	double total_thickness=0;
	double emissivity=0.8;	//PCB resin material
	
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	
	//First we need to determine the thickness of just the substrate itself. 
	//We only know the ENTIRE package thickness (not the individual portions)
	
	//compute the total thickness of the sample stackup
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++)
		total_thickness+=datalibrary_->config_data_->fcpbga_layer_thickness_[i];
		
	//store the thickness of the substrate from the sample stackup
	double substrate_layer_total_thickness=total_thickness;
	
	//finish accumulating the total thickness of the substrate+PCB regions
	for(int i=0;i<(int)datalibrary_->config_data_->pwb_layer_thickness_.size();i++)
		total_thickness+=datalibrary_->config_data_->pwb_layer_thickness_[i];
	
	double percent=substrate_layer_total_thickness/total_thickness;	//this is the percent of the stackup that's the substrate layer
	
	//Calculate the actual thickness of the substrate layer by assuming that is the same proportions as the sample stackup
	if(layer.thickness_==0)layer.thickness_=thickness*percent;							//this is the actual thickness of the substrate
	
	double volume=width*height*layer.thickness_;
	
	//scale the layer thicknesses based upon the relative thickness of the package substrate actually used
	vector<double> layer_thickness;
	
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++){
		//this is the percentage of the substrate that this layer occupies
		double percentage=datalibrary_->config_data_->fcpbga_layer_thickness_[i]/substrate_layer_total_thickness;
		layer_thickness.push_back(layer.thickness_*percentage);	//determine the actual layer thickness
	}

	//calculates the equivalent specific heat for the entire layer 
	//this is SUM(i=1 to n, %volume_i * specific_heat_i)
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++){
		specific_heat += ((width*height*layer_thickness[i])/(width*height*layer.thickness_) * datalibrary_->config_data_->fcpbga_layer_specific_heat_[i]);			
		}
		
	layer.spec_heat_=specific_heat;

	double density=0;
	//calculates the equivalent density for the entire layer 
	//this is SUM(i=1 to n, %volume_i * specific_heat_i)
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++){
		density += ((width*height*layer_thickness[i])/(width*height*layer.thickness_) * datalibrary_->config_data_->fcpbga_layer_density_[i]);			
		}
		
	layer.density_=density;

	//compute the equivalent vertical resistance across the package substrate layer
	double resistance=0;
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++){
		resistance+=(layer_thickness[i]/
					 (datalibrary_->config_data_->fcpbga_layer_conductivity_vert_[i]*width*height));
	}
	
   double conductivity = (layer.thickness_ / (width*height*resistance) );
	
	layer.vertical_conductivity_down_=conductivity;
	layer.vertical_conductivity_up_=conductivity;
	
	//this calculates the equivalent resistance for the entire layer (horizontal)
	
	conductivity=0;
	for(int i=0;i<(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();i++){
		conductivity+= datalibrary_->config_data_->fcpbga_layer_conductivity_horiz_[i];
	}
    conductivity/=(int)datalibrary_->config_data_->fcpbga_layer_thickness_.size();
   layer.horizontal_conductivity_=conductivity;   
   
   layer.emissivity_=emissivity;
   layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER)|(1<<HEAT_RADIATION_TRANSFER);
}


//CALCULATE PINS LAYER PROPERTIES
void sesctherm3Dmaterial::calc_pins_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	int num_pins=(int)datalibrary_->config_data_->pin_count_;
	double package_area=datalibrary_->config_data_->package_width_*datalibrary_->config_data_->package_height_;
	double pin_tip_area=3.14159265*powf(datalibrary_->config_data_->pin_pitch_,2);
	double pin_height=datalibrary_->config_data_->pins_height_;
	double conductivity=datalibrary_->layer_materials_.find("COPPER").conductivity_;
	double spec_heat=datalibrary_->layer_materials_.find("COPPER").spec_heat_;
	double density_copper=datalibrary_->layer_materials_.find("COPPER").density_;
	double r_pin=pin_height/(conductivity*pin_tip_area);
	double r_effective=r_pin/num_pins;	//parallel resistance
	double emissivity=datalibrary_->layer_materials_.find("COPPER").emissivity_;
	
	//store conductivity
	double cond_eff=(conductivity*pin_tip_area*num_pins)/(pin_height);
	
	//store cond_eff to the pins layer
	layer.vertical_conductivity_up_=cond_eff;
	layer.vertical_conductivity_down_=cond_eff;
	layer.horizontal_conductivity_=0;			//we assume that there is negligible heat transfer from one pin to another
	
	//calculate the effective specific heat
	//effective specific heat = specific heat of copper * (% area copper versus air)
	//store effective specific heat to pins layer
	double percent_copper=(num_pins*pin_tip_area)/(package_area);
	
	layer.density_=density_copper*percent_copper;
	layer.spec_heat_=spec_heat*percent_copper;	
	
	//store height
	if(layer.height_==0)layer.height_=datalibrary_->config_data_->package_height_;
	
	//store width
	if(layer.width_==0)layer.width_=datalibrary_->config_data_->package_width_;
	
	//store thickness
	if(layer.thickness_==0)layer.thickness_=pin_height;
	
	layer.emissivity_=emissivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONDUCTION_TRANSFER);
}


//CALCULATE OIL LAYER PROPERTIES
//NOTE: we assume oil flow is along WIDTH of chip (NOT HEIGHT)
 void sesctherm3Dmaterial::calc_oil_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double density=datalibrary_->config_data_->coolant_density_;
	double viscosity=datalibrary_->config_data_->coolant_viscosity_;
	double conductivity=datalibrary_->config_data_->coolant_thermal_conductivity_;
	double spec_heat=datalibrary_->config_data_->coolant_specific_heat_;
	double prandtl_number=datalibrary_->config_data_->coolant_prandtl_number_;
	double flow_rate=datalibrary_->config_data_->coolant_flow_rate_;
	double angle=datalibrary_->config_data_->coolant_angle_;
	double nozzle_diameter=datalibrary_->config_data_->coolant_nozzle_diameter_;
	double coverage_percent=datalibrary_->config_data_->coolant_coverage_percent_;
	double height=datalibrary_->config_data_->chip_height_;
	double width=datalibrary_->config_data_->chip_width_; //NOTE: we assume oil flow is along WIDTH of chip (the longer part) (NOT HEIGHT)
	
	//compute thickness of oil across chip as a function of the angle and the nozzle diameter
	//Note: this is not the actual thickness of the oil on the chip, but instead an "effective thermal capacitance" computation for the oil volume
	double thickness=(nozzle_diameter*powf(cos(angle),2) )/2;	
	
	//compute the volume of oil flowing over the chip
	double eff_oil_volume=thickness*height*width;
	
	//based upon the volume flow rate of the nozzle and that of the oil flowing over the chip, compute the effective velocity
	//this is augmented by coverage_percent because we assume that a certain amount of the flow is lost
	//this is flow_rate_nozzle/eff_oil_volume
	//this is the speed at which the oil is flowing over the chip
	double eff_velocity=coverage_percent*flow_rate/eff_oil_volume;
	
	//compute the reynold's number
	//if the flow becomes non-laminar, it is NOT modeled (quit in this case)
	 double r_el=(eff_velocity*width)/viscosity;
	 
	 if(r_el>20000){
		cerr << "Fatal: sesctherm3Dmaterial::calc_oil_layer_properties -> Reynold's Number is [" << r_el << "]. This indicates turbulent flow. This is not modeled!" << endl;
		exit(1);
	 }

	double nusselt_average=(conductivity/width)*0.664*sqrtf(r_el)*powf(prandtl_number,.3333333333);
	double average_convection_coef=(nusselt_average*conductivity)/width;
	 
	//local convection coeff is 1/2 the average one in this case
	double local_convection_coef = average_convection_coef/2;
	
	datalibrary_->config_data_->oil_layer_convection_coefficient_=local_convection_coef;
	datalibrary_->config_data_->oil_used_=true;
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	layer.spec_heat_=spec_heat;
	layer.density_=density;
	if(layer.thickness_==0)layer.thickness_=thickness;
	layer.horizontal_conductivity_=conductivity;
	layer.vertical_conductivity_down_=conductivity;
	layer.vertical_conductivity_up_=conductivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONVECTION_TRANSFER);
	layer.convection_coefficient_=average_convection_coef;
	#ifdef _SESCTHERM_DEBUG
	double heat_transfer_rate_local=(width)*(30)*(local_convection_coef);
	cerr << "[DEBUG INFO] sesctherm3Dmaterial::calc_oil_layer_properties() => Local Heat Transfer Rate (for 30K drop) is [" << local_convection_coef << "]" << endl;
	 #endif
}



//CALCULATE SILICON LAYER PROPERTIES
 void sesctherm3Dmaterial::calc_silicon_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double height=datalibrary_->config_data_->chip_height_;
	double width=datalibrary_->config_data_->chip_width_;
	double chip_thickness=datalibrary_->config_data_->chip_thickness_;
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	layer.density_=datalibrary_->layer_materials_.find("BULK_SI").density_;
	layer.spec_heat_=datalibrary_->layer_materials_.find("BULK_SI").spec_heat_;
	//Silicon conductivity is temperature-dependent
	//117.5 - 0.42*(T-100) is what is typically used (FLOTHERM)
	double tmp=117.5-0.42*(datalibrary_->config_data_->init_temp_-100);
	layer.horizontal_conductivity_=117.5-0.42*(datalibrary_->config_data_->init_temp_-100);
	layer.vertical_conductivity_down_=117.5-0.42*(datalibrary_->config_data_->init_temp_-100);
	layer.vertical_conductivity_up_=117.5-0.42*(datalibrary_->config_data_->init_temp_-100);
	
	//what is the %of the sample stackup that the substrate layer occupies?
	//this percentage is used to find the actual thickness of the silicon substrate for the chip_thickness
	double t_sub=datalibrary_->config_data_->sample_silicon_substrate_thickness_;
	double t_box=datalibrary_->config_data_->sample_box_thickness_;
	double t_si=datalibrary_->config_data_->sample_silicon_film_thickness_;
	int technology = datalibrary_->config_data_->technology_;
	double t_interconnect=0;
	int num_layers = (int)sesctherm3Ddatalibrary::technology_parameters_[TECH_LEVELS][technology];
	for(int i=0;i<num_layers;i++){
			t_interconnect = sesctherm3Ddatalibrary::technology_parameters_[10+(i*9)+TECH_H][technology]; //get the thickness of the layer for the given technology
	}
	
	double percentage_silicon_substrate=t_sub/(t_sub+t_box+t_si+t_interconnect);	//t_sub/(t_sub+t_box+t_si)
	if(layer.thickness_==0)layer.thickness_=percentage_silicon_substrate*chip_thickness;
	layer.heat_transfer_methods_=(1<<HEAT_RADIATION_TRANSFER)|(1<<HEAT_CONDUCTION_TRANSFER);
	layer.emissivity_=datalibrary_->layer_materials_.find("BULK_SI").emissivity_;
}


//This is based upon "Wiring Statistics and Printed Wiring Board Thermal Conductivity" Richard D Nelson, Teravicta Technologies, IEEE SEMI-THERM 2001
//Data Taken from "summary of thermal conductivity results for full PWB models"
//Wire Width: 8mills, Wire thickness: 1mill
//Dielectric thickness: 6mills
//Lateral space between wires 16mills
//Copper Via Drill Diameter: 8mills
//Copper Vias Plating Thickness: 1mill
//Minimum Wire Length/Spacing: 3.6mm
//Maximum Wire Length/Spacing: 7.2mm
//Percent Copper in entire PWB: 6.76%
//Kz=.844W/mk
//Kx,y=16.5W/mk
//12"x9.6" (inches)
void sesctherm3Dmaterial::calc_pcb_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	layer.horizontal_conductivity_=16.5;
	layer.vertical_conductivity_up_=.844;
	layer.vertical_conductivity_down_=0;
	layer.heat_transfer_methods_=(1<<HEAT_RADIATION_TRANSFER)|(1<<HEAT_CONDUCTION_TRANSFER);
	if(layer.thickness_==0)	layer.thickness_=.0016;		//1.6mm thickness
	if(layer.height_==0)	layer.height_=.24384;			//243.84cm height
	if(layer.width_==0)	layer.width_=.3048;			//304.8cm width
	layer.emissivity_=datalibrary_->layer_materials_.find("COPPER").emissivity_*.0676 + 0.8*.9324;
	layer.spec_heat_=datalibrary_->layer_materials_.find("COPPER").spec_heat_*.0676 + 1200*.9324;
	layer.density_=datalibrary_->layer_materials_.find("COPPER").density_*.0676 + 2080*.9324;
}





//CALCULATE AIR LAYER PROPERTIES
//NOTE: we assume oil flow is along WIDTH of chip (NOT HEIGHT)
// Air is assumed to be at 20 degrees celcius (room temperature)
void sesctherm3Dmaterial::calc_air_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_){
	double density=1.205; //1.205kg/m^3
	double conductivity=0.0257; //.0257W/m*k
	double spec_heat=1005;	//1.005kg/m^3
	double height=datalibrary_->config_data_->chip_height_;
	double width=datalibrary_->config_data_->chip_width_; //NOTE: we assume oil flow is along WIDTH of chip (the longer part) (NOT HEIGHT)
	
	//assume that there is 1m thickness of air above the chip
	double thickness=1;	
	
	//This approximation is taken from the "Heat Transfer Handbook"
	layer.convection_coefficient_=3.886*sqrt(datalibrary_->config_data_->fan_velocity_/width);
	if(layer.width_==0)layer.width_=width;
	if(layer.height_==0)layer.height_=height;
	layer.spec_heat_=spec_heat;
	layer.density_=density;
	if(layer.thickness_==0)	layer.thickness_=thickness;
	layer.horizontal_conductivity_=conductivity;
	layer.vertical_conductivity_down_=conductivity;
	layer.vertical_conductivity_up_=conductivity;
	layer.heat_transfer_methods_=(1<<HEAT_CONVECTION_TRANSFER);
#ifdef _SESCTHERM_DEBUG
	double heat_transfer_rate_local=(width)*(30)*(layer.convection_coefficient_);
	cerr << "[DEBUG INFO] sesctherm3Dmaterial::calc_air_layer_properties() =>  Heat Transfer Rate (for 30K drop) is [" << heat_transfer_rate_local << "]" << endl;
#endif
}

