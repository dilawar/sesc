
class sesctherm3Dmaterial;
class sesctherm3Dmaterial_list;




class sesctherm3Dmaterial{
public:
	sesctherm3Dmaterial(){}
	//These are separate equations for recomputing the conductivity of the materials based upon temperature
   sesctherm3Dmaterial(string& name, double density, double spec_heat, double conductivity, double emmisivity);
	
	static void calc_air_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_pcb_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_silicon_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_silicon_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_oil_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_pins_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_package_substrate_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_pwb_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_c4underfill_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_heat_spreader_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_heat_sink_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void  calc_heat_sink_fins_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_interconnect_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_interconnect_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_);

	static void calc_die_transistor_layer_properties(sesctherm3Dlayerinfo& layer, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_die_transistor_layer_properties_dyn(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_material_properties_dyn_density(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_);
	
	static void calc_material_properties_dyn_temp(int layer, int32_t x, int32_t y, sesctherm3Ddatalibrary* datalibrary_);
	
	static double calc_effective_interconnect_layer_horizontal_conductivity(int material_layer, int32_t layer, double density, sesctherm3Ddatalibrary* datalibrary_);
	
	static double calc_effective_interconnect_layer_vertical_conductivity(int layer, double density, sesctherm3Ddatalibrary* datalibrary_);
	
	static double calc_interlayer_conductivity(double dielectric_constant, int32_t material, sesctherm3Ddatalibrary* datalibrary_);

	string name_;					//name of material
    double density_;				//density (row) kg/m^3
    double spec_heat_;				//specific heat (c) (J/kg*K)
    double conductivity_;			//conductivity (k) (W/m*K)
	double emissivity_;
	int type_;						//defined based upon sesctherm3Ddefine.h
	
	
		friend ostream& operator<<(ostream & os, const sesctherm3Dmaterial& tmpmaterial){

		os << "Material:[" << tmpmaterial.name_ << "] Density:[" << tmpmaterial.density_ << "]" << endl;
		os << "Material:[" << tmpmaterial.name_ << "] Specific Heat:[" << tmpmaterial.spec_heat_ << "]" << endl;
		os << "Material:[" << tmpmaterial.name_ << "] conductivity:[" << tmpmaterial.conductivity_ << "]" << endl;
		return(os);
	}
	
	
	
};

class sesctherm3Dmaterial_list : public sesctherm3Dmaterial{
public:
	sesctherm3Dmaterial_list(){
	}
	void create(string& name, double density, double spec_heat, double conductivity, double emmisivity);
    sesctherm3Dmaterial& find(string name);
	friend ostream& operator<<(ostream &os, sesctherm3Dmaterial_list &tmpmaterial_list) {
		std::map<string, sesctherm3Dmaterial *>::iterator it = tmpmaterial_list.storage.begin();
		
		while(it != tmpmaterial_list.storage.end()){
			os << it->second;
		}
		return(os);
	}
	std::map<string, sesctherm3Dmaterial *> storage;
};
