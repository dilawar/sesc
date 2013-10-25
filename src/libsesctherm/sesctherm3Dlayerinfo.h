
#ifndef Sesctherm3Dlayerinfo_H
#define Sesctherm3Dlayerinfo_H

#include "estl.h"

class sesctherm3Dlayerinfo{

	public:
		sesctherm3Dlayerinfo(sesctherm3Ddatalibrary* datalibrary, int32_t num_layers);
		~sesctherm3Dlayerinfo();
		void determine_layer_properties();
		void offset_layer();
		std::vector<double> compute_average_temps(int flp_layer);
		std::vector<double> compute_average_powers(int flp_layer);		
		std::vector<double> compute_energy();
		
		static void allocate_layers(sesctherm3Ddatalibrary* datalibrary_);
		
		//this computes the average of both temperature and power across all of the layers
		static dynamic_array<model_unit> compute_layer_averages(std::vector<int>& layers, sesctherm3Ddatalibrary* datalibrary_);
		static dynamic_array<model_unit> compute_layer_diff(int layer1, int32_t layer2, sesctherm3Ddatalibrary* datalibrary_);

		
			void print(bool detailed);
			void print_lumped_metrics();
		//static void sesctherm3Dlayerinfo::compute_offsets(int layer_num, sesctherm3Ddatalibrary* datalibrary_);
		
		void accumulate_data_for_sample();
		void compute_sample();
		int flp_num_;
		int layer_number_;
		string name_;
		int type_;		//the layer types are defined in the sesctherm3Ddefine.h
		double thickness_;
		
		double height_;
		double width_;
		double leftx_;		//calculated 
		double bottomy_;	//calculated
		double rightx_;
		double topy_;
		
		//these are the effective material properties for the layer (averaged)
		double horizontal_conductivity_;
		double vertical_conductivity_down_;
		double vertical_conductivity_up_;
		double spec_heat_;
		double density_;
		double convection_coefficient_;		//used for fluid/air layers
		double emissivity_;
		
		//These are binary values, use DEFINES: HEAT_CONVECTION_TRANSFER, HEAT_CONDUCTION_TRANSFER, HEAT_EMMISION_TRANSFER for transfer types
		uint32_t heat_transfer_methods_;
			
		double granularity_;
		
		chip_floorplan* chip_floorplan_;	//if this is a chip layer, this will be defined, NULL otherwise
		ucool_floorplan* ucool_floorplan_;	//if this is a ucool layer, this will be defined, NULL otherwise
  
		sesctherm3Ddatalibrary* datalibrary_;			//link to the datalibrary
			
	    std::vector< std::vector< model_unit * > > located_units_;
		dynamic_array<model_unit>* floorplan_layer_dyn_;	//this is the dynamic array which corresponds to this layer
		int unused_dyn_count_;								//this stores the number of unused dynamic array entries
		

		map<double,int> coord_to_index_x_;
		map<double,int> coord_to_index_y_;
		
		vector<double> temperature_map_;	//this is the temperature map for the layer (related to chip/ucool floorplan)
											//this is only currently used for the transistor layer
											//these are temperatures averaged by flp unit
		vector<double> power_map_;
		
		dynamic_array<double> running_average_temperature_map_;		//this is used to create the running average temperatures
		dynamic_array<double> running_average_energy_map_;
		dynamic_array<double> running_max_temperature_map_;
		dynamic_array<double> running_min_temperature_map_;
		dynamic_array<double> running_max_energy_map_;
		dynamic_array<double> running_min_energy_map_;
		dynamic_array<double> sample_temperature_map_;				//this is the average temps over the sample period
		
		bool layer_used_;					//indicates if the layer is used in the stackup
		
		int num_layers_;
		
		bool temp_locking_enabled_;					//lock the temperature to lock_temp_
		double lock_temp_;
private:

			class temporary_layer_info{
			public:
				struct cmpGranularityThenSize {
					bool operator()(const temporary_layer_info& a, const temporary_layer_info& b) const {
						if(EQ(a.granularity_,b.granularity_)){
							return(LT(a.size_,b.size_));
						}
						else 
							return(LT(a.granularity_,b.granularity_));
					}
				};				
				temporary_layer_info(int layer_num, double granularity, double height, double width, double size, int32_t nregions, double areas, double areas_removed){
					layer_num_=layer_num;
					granularity_=granularity;
					height_=height;
					width_=width;
					size_=size;
					nregions_=nregions;
					areas_=areas;
					areas_removed_=areas_removed;
					
				}
				int layer_num_;
				double granularity_;
				double height_;
				double width_;
				double size_;
				int nregions_;
				double areas_;
				double areas_removed_;	
			};
};

#endif
