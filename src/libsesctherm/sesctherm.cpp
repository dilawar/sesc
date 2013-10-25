
#include "sesctherm3Dinclude.h"

#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

void showUsage() {
	
	printf("sesctherm:\n");
	printf("\t-i <infile>             - The therm file for which you want temperature data\n");
	printf("\t-o <outfile>(optional)  - Output file name for either steady state or transient temperature\n");
	printf("\t-c <power.conf>         - This is the mappings from floorplan variables to sesc variables\n");
	printf("\t-s <sim_point.labels>   - Simpoint labels file\n");
	printf("\t-t <starting_name>      - Simpoint therm trace starting filename. E.g: crafty_X\n");
	printf("\t-p <label path>         - Path to simpoint therm traces\n");
	
}

const char *conf_file  = NULL;
const char *input_file = NULL;
const char *flp_file   = NULL;
const char *output_file= NULL;
const char *label_file = NULL;
const char *label_match= NULL;
const char *label_path = ".";

void read_parameters(int argc, char **argv) {
	int opt;
	
	while ((opt = getopt(argc, argv, "c:f:i:o:s:t:p:")) != -1) {
		switch (opt) {
			case 'c':
				conf_file = optarg;
				break;
			case 'f':
				flp_file = optarg;
				break;
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			case 's':
				label_file = optarg;
				break;
			case 't':
				label_match = optarg;
				break;
			case 'p':
				label_path = optarg;
				break;
			default:
				showUsage();
				exit(1);
				break;
		}
	}
	
	if ((conf_file==NULL || input_file=="")  && label_file == 0) {
		showUsage();
		exit(1);
	}
	
	if (label_file && !label_match) {
		cerr << "ERROR: you should specify -t matching name when simpoints are active\n";
		showUsage();
		exit(2);
	}
	
}

int process_trace(const char *input_file
				  ,sesctherm3Dmodel &temp_model
				  ,double power_timestep
				  ,int transistor_layer
				  ,double initialTemp
				  ,int power_samples_per_therm_sample
				  ,bool rabbit
				  ,int cyclesPerSample
				  ) {
	
	int num_computations = 0;
	// ----------------------------------------------------------------
	// READ trace
	ThermTrace trace(input_file);
	if (!trace.is_ready())
		return 0; // trace file did not exist
	
	static std::vector<double> power(trace.get_energy_size());
	
	static int32_t pending_power_samples_from_prev_trace = 0;
	static int32_t samples=0;
	
	while(trace.read_energy()) {
		//-------------------------------------
		// Advance Power
		if (pending_power_samples_from_prev_trace<=0) {
			for(size_t j=0;j<trace.get_energy_size();j++) {
				power[j] = trace.get_energy(j);
			}
			
			samples = 1;
			pending_power_samples_from_prev_trace = power_samples_per_therm_sample-1;
		}
		
		while(pending_power_samples_from_prev_trace > 0 || rabbit) {
			if (!trace.read_energy())
				break;
			
			for(size_t j=0;j<trace.get_energy_size();j++) {
				power[j] += trace.get_energy(j);
			}
			
			samples++;
			pending_power_samples_from_prev_trace--;
			
			if (rabbit && samples > 20*power_samples_per_therm_sample) {
				pending_power_samples_from_prev_trace = 0;
				break;
			}
		}
		
#if 0
		cout << "P " << temp_model.get_time();
		double total=0;
		double p;
		for(size_t j=0;j<power.size();j++) {
			p = power[j]/samples;
			cout << p << "\t";
			total += p;
		}
		cout << "\t:\t" << total << endl;
		cout << ": " << samples << endl;
#endif
		
		for(size_t j=0;j<trace.get_energy_size();j++) {
			temp_model.set_power_flp(j, transistor_layer,  power[j]/samples);
			power[j] = 0.0;
		}
		
		//-------------------------------------
		// Advance Thermal
		
		double ts = power_timestep*samples;
		temp_model.compute(ts, true);
		
		num_computations++;
		
		samples = 0;
		
		//-------------
		static double print_at = 0;
		if (temp_model.get_time() > print_at ) {
			print_at = temp_model.get_time() + 0.005; // every 5ms
													  //	temp_model.print_graphics();
			
			if (rabbit)
				cout << "R " << temp_model.get_time() << " ";
			else
				cout << "T " << temp_model.get_time() << " ";
			
			std::vector<double> flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer);
			for(uint32_t i=0;i<flp_temperature_map.size();i++){
				cout << flp_temperature_map[i] << "\t";
			}
			cout << endl;
		}
	}
	
	return num_computations;
}		

int main(int argc, char **argv) {
	
	clock_t start, end;
	double elapsed=0;
	
	read_parameters(argc, argv);
	
	SescConf = new SConfig(conf_file);
	if (SescConf == 0) {
		cout << "Cannot open configuration file" << endl;
		exit(1);
	}
	
	cout << "STARTING SESCTHERM" << endl;
	
	start = clock();
	sesctherm3Dmodel temp_model(flp_file,0,output_file,true,false);	
	end = clock();
	
	cout << "MODEL INITIALIZED" << endl;
	
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	cout << "MODEL INITIALIZATION TIME IS " << elapsed << "SECONDS" << endl;
	
	cout << "MAX MODEL TIMESTEP:" << temp_model.get_max_timestep() << endl;
	cout << "RECOMMENDED MODEL TIMESTEP: " << temp_model.get_recommended_timestep() << endl;
	
	//this is a map of maps
	//it stores the time index of the run, and the corresponding temperature map, where flp_temperature_map[1.0][0] returns the temperature for flp unit 0 at timestep 1.0 
	
	const char *model   = SescConf->getCharPtr("thermal","model");
	double initialTemp  = SescConf->getDouble(model,"initialTemp"); // sesctherm works in C (not K)
	double ambientTemp  = SescConf->getDouble(model,"ambientTemp");
	
	int cyclesPerSample = SescConf->getInt(model,"CyclesPerSample");
	double frequency    = SescConf->getDouble("technology","Frequency");
	
	int power_samples_per_therm_sample = SescConf->getInt(model,"PowerSamplesPerThermSample");
	double power_timestep = cyclesPerSample/frequency;
	double therm_timestep = power_timestep * power_samples_per_therm_sample;
	
	cout << "USED THERM TIMESTEP: " << therm_timestep << " POWER TIMESTEP: " << power_timestep << endl;
	cout << "USED power_samples_per_therm_sample: " << power_samples_per_therm_sample << endl;
	
	const char *thermal = SescConf->getCharPtr("","thermal");
	int n_layers_start  = SescConf->getRecordMin(thermal,"layer");
	int n_layers_end    = SescConf->getRecordMax(thermal,"layer"); // last layer has ambient
	int transistor_layer=-1;
	for(int i = n_layers_start ; i <= n_layers_end ; i++) {
		const char *layer_name = SescConf->getCharPtr(thermal,"layer",i);
		const char *layer_sec  = SescConf->getCharPtr(layer_name,"type");
		if (strcmp(layer_sec,"die_transistor")==0)
			transistor_layer = i;
		
		
		if (SescConf->getInt(layer_name,"lock_temp") == -1 )
			temp_model.set_temperature_layer(i,initialTemp);
	}
	// temp_model.set_temperature_layer(n_layers_end,ambientTemp); FIXME: this crashes n_layers_end == 8
	
	if (transistor_layer == -1) {
		for(int i = n_layers_start ; i <= n_layers_end ; i++) {
			const char *layer_name = SescConf->getCharPtr(thermal,"layer",i);
			const char *layer_sec  = SescConf->getCharPtr(layer_name,"type");
			if (strcmp(layer_sec, "bulk_silicon")==0)
				transistor_layer = i;
			
			if (SescConf->getInt(layer_name,"lock_temp") == -1 )
				temp_model.set_temperature_layer(i,initialTemp);
		}
		if (transistor_layer == -1){
			cerr << "ERROR: Could not find 'die_transistor' layer" << endl;
			exit(-2);
		}
	}
	else{
		cerr << "USING FLOORPLAN WITHIN LAYER:" << transistor_layer << endl;
	}
	
	int n_blocks = 0;
	for(int i = n_layers_start ; i <= n_layers_end ; i++)
		n_blocks += temp_model.get_modelunit_count(i);
	
	cout << "Modeling " << n_blocks << " blocks" << endl;
	SescConf->lock(); // Done reading all the options
	
	elapsed=0;
	
	int num_computations = 0;
	
	if (input_file) {
		
		start = clock();	
		num_computations = process_trace(input_file
										 ,temp_model
										 ,power_timestep
										 ,transistor_layer
										 ,initialTemp
										 ,power_samples_per_therm_sample
										 ,false
										 ,cyclesPerSample
										 );
		end = clock();
		elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
	}else if (label_file) {
		
		std::ifstream file;
		file.open(label_file);
		if (!file.is_open() || file.bad()) {
			std::cerr << "ERROR: label file \""<< label_file <<"\" can't be found or open\n";
			exit(-1);
		}
		
		char line[1024];
		while(!file.getline(line, 1000).eof()) {
			std::stringstream line_tmp;
			
			line_tmp.clear();
			line_tmp.str(line);
			
			int phaseID;
			line_tmp >> phaseID;
			
			cout << "Phase " << phaseID << endl;
			
			DIR *pdir;
			struct dirent *pent;
			
			pdir=opendir(label_path); //"." refers to the current dir
			if (!pdir) {
				printf ("opendir() failure; terminating");
				exit(1);
			}
			errno=0;
			while ((pent=readdir(pdir))) {
				if (temp_model.get_time() > 120 ) { // Just the first 2 minutes
					MSG("Warning: Reached the 2 minutes simulation limit");
					break;
				}
				
				char cadena[1024];
				sprintf(cadena,"sesc_X%d_%s.therm",phaseID,label_match);
				if (strncmp(pent->d_name,cadena, strlen(cadena))==0) {
					
					//-------------------------
					
					bool rabbit=false;
					
					static std::set<int> sampled;
					if (sampled.find(phaseID) == sampled.end()) {
						sampled.insert(phaseID);
					}else if (therm_timestep > (power_timestep * 8)) {
						continue;
						//	    rabbit = true;
					}
					
					sprintf(cadena,"%s/%s",label_path,pent->d_name);
					
					start = clock();	
					num_computations += process_trace(cadena
													  ,temp_model
													  ,power_timestep
													  ,transistor_layer
													  ,initialTemp
													  ,power_samples_per_therm_sample
													  ,rabbit
													  ,cyclesPerSample
													  );
					end = clock();
					elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
					
					//-------------------------
				}
			}
			if (errno) {
				printf ("readdir() failure; terminating");
				exit(1);
			}
			closedir(pdir);    
			
		}     
		
		file.close();
		
		
	}else if (argc>6) {
		// sesctherm -c power.conf -o ouput input input input input input ...
		// sesctherm -c power.conf -o ouput -i input
		num_computations = 0;
		
		for(int i=5;i<argc;i++) {
			cout << "Processing trace " << argv[i] << endl;
			start = clock();
			
			num_computations += process_trace(argv[i]
											  ,temp_model
											  ,power_timestep
											  ,transistor_layer
											  ,initialTemp
											  ,power_samples_per_therm_sample
											  ,false
											  ,cyclesPerSample
											  );
			
			end = clock();
			elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
		}
	}else{
		std::vector<double> flp_temperature_map;
		double timestep, recommended_timestep;
		//simulate output for Delphi thermal test chip trace pulsed 8.862W
		
		temp_model.datalibrary_->time_=0.0;
		
		//initialize chip temperature to 18.33 degrees celcius (65 farenheit)
		double init_temp=18.33;
		temp_model.set_temperature_layer(0,	init_temp);		//mainboard (locked)
		temp_model.set_temperature_layer(1, init_temp);		//interconnect
		temp_model.set_temperature_layer(2, init_temp);		//die_transistor
		temp_model.set_temperature_layer(3, init_temp);		//bulk silicon
		temp_model.set_temperature_layer(4, init_temp);		//air (locked)		
		
		
		/************ Testing print of temperatures on the air layer
		
		recommended_timestep=temp_model.get_max_timestep();	 
		timestep=.5;
				num_computations=timestep/min(.1*timestep,recommended_timestep);
		cout << min(.1*timestep,recommended_timestep) << endl;
		exit(1);
		
		//set power to zero
		temp_model.set_power_flp(4, transistor_layer,  3.75); //set the silicon layer power generation to 3.75W
		
		num_computations=10;		
		for(int j=0;j<num_computations;j++){
			temp_model.compute(recommended_timestep, true);
			for (int itor_x=0;itor_x<(int)temp_model.datalibrary_->all_layers_info_[4]->floorplan_layer_dyn_->ncols();itor_x++) {
				for (int itor_y=0;itor_y<(int)temp_model.datalibrary_->all_layers_info_[4]->floorplan_layer_dyn_->nrows();itor_y++) {
					if ((*temp_model.datalibrary_->all_layers_info_[4]->floorplan_layer_dyn_)[itor_x][itor_y].defined_==false)
						continue;
					cout << *(*temp_model.datalibrary_->all_layers_info_[4]->floorplan_layer_dyn_)[itor_x][itor_y].get_temperature() << "\t";
				}
				cout << endl;	
			}
		}
				cout <<	endl << temp_model.get_temperature_layer(transistor_layer,4)[4] << endl;
		exit(1);
		********** END OF Testing print of temperatures on the air layer  *******/
		
		recommended_timestep=temp_model.get_max_timestep();	  
		//set power to zero
		temp_model.set_power_flp(4, transistor_layer,  0); //set the silicon layer power generation to zero
		timestep=.5;		//power pulse starts at 8.1s
		
		num_computations=timestep/min(.1*timestep,recommended_timestep);
		for(int j=0;j<num_computations;j++){
			if(j%(int)(.25*num_computations)==0)
				temp_model.compute(min(.1*timestep,recommended_timestep), true);
			else
				temp_model.compute(min(.1*timestep,recommended_timestep), false);
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer);
			cout << temp_model.get_time() << " ";
			cout << flp_temperature_map[4]-init_temp << " ";
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,0);
			cout << flp_temperature_map[4] << endl;
		}
		
		temp_model.set_power_flp(4, transistor_layer,  3.75); //set the silicon layer power generation to 3.75W  
		timestep=15.5;		//power pulse ends at 12.6s
							//timestep=.85;
							// recommended_timestep=temp_model.get_recommended_timestep();
		num_computations=timestep/min(.1*timestep,recommended_timestep);
		for(int j=0;j<num_computations;j++){
			if(j%(int)(.5*num_computations)==0){

				temp_model.compute(min(.1*timestep,recommended_timestep), true);
			}
			else{
				temp_model.compute(min(.1*timestep,recommended_timestep), false);
				}
				
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer);
			cout << temp_model.get_time() << " ";
			cout << flp_temperature_map[4]-init_temp << " ";
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,0);
			cout << flp_temperature_map[4] << endl;
		}	  
		
		temp_model.set_power_flp(4, transistor_layer,  0); //set the silicon layer power generation to 0W	  
		timestep=28.5;		//power pulse starts again at 18.3s
							// recommended_timestep=temp_model.get_recommended_timestep();
		num_computations=timestep/min(.1*timestep,recommended_timestep);
		for(int j=0;j<num_computations;j++){ 
			if(j%(int)(.5*num_computations)==0){
							cerr << j << "/" << num_computations << endl;
				temp_model.compute(min(.1*timestep,recommended_timestep), true);
			}
			else
				temp_model.compute(min(.1*timestep,recommended_timestep), false);
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer);
			cout << temp_model.get_time() << " ";
			cout << flp_temperature_map[4]-init_temp << " ";
			flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,0);
			cout << flp_temperature_map[4] << endl;
		}
		
		// ----------------------------------------------------------------
		// SYNTHETIC TEST
		/*
		 std::vector<double> flp_temperature_map;
		 
		 num_computations = 100;
		 // temp_model.compute_rc_start();
		 
		 for(int j=0;j<num_computations;j++){
			 for(uint32_t i=0;i<temp_model.chip_flp_count(transistor_layer);i++) {
				 temp_model.set_power_flp(i, transistor_layer,  1); //storing data for chip layer (layer 5 in this stackup)
			 }
			 //	temp_model.set_temperature_layer(8,22);
			 temp_model.set_temperature_layer(0,27);
			 start = clock();	
			 temp_model.compute(therm_timestep);		//compute with some timestep	
			 end = clock();
			 elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
			 temp_model.print_graphics();
			 
			 cout << "********************************* TIMESTEP [ " << temp_model.get_time() << " ] *********************************" << endl;
			 flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer); // FIXME: not sure if the source_layer is 5
			 for(uint32_t i=0;i<flp_temperature_map.size();i++){
				 cout << flp_temperature_map[i] << "\t";
			 }
			 cout << endl;
		 }
		 */
		//temp_model.compute_rc_stop();
		
		// temp_model.set_time(0.0);
		//cout << "COMPUTATION TIME FOR " << num_computations << " ITERATIONS IS " << elapsed << "  (" << elapsed/num_computations << "SECONDS PER ITERATION )" << endl;
		/*
		 temp_model.set_temperature_layer(0,27);
		 temp_model.set_temperature_layer(1,30);
		 temp_model.set_temperature_layer(2,30);
		 temp_model.set_temperature_layer(3,30);
		 temp_model.set_temperature_layer(4,30);
		 temp_model.set_temperature_layer(5,30);
		 temp_model.set_temperature_layer(transistor_layer,30);
		 temp_model.set_temperature_layer(7,30);
		 //  temp_model.set_temperature_layer(8,22);
		 
		 num_computations = 10;
		 for(int j=0;j<num_computations;j++){
			 for(uint32_t i=0;i<temp_model.chip_flp_count(transistor_layer);i++) {
				 temp_model.set_power_flp(i, transistor_layer,  1); //storing data for chip layer (layer 5 in this stackup)
			 }
			 //	  temp_model.set_temperature_layer(8,22);
			 temp_model.set_temperature_layer(0,27);
			 start = clock();
			 temp_model.fast_forward(therm_timestep*100);	//compute for 1ms (fast forward using effective RC time constant
			 end = clock();
			 elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
			 //temp_model.print_graphics();
			 
			 cout << "********************************* TIMESTEP [ " << temp_model.get_time() << " ] *********************************" << endl;
			 flp_temperature_map=temp_model.get_temperature_layer(transistor_layer,transistor_layer); // FIXME: not sure if the source_layer is 5
			 for(uint32_t i=0;i<flp_temperature_map.size();i++) {
				 cout << flp_temperature_map[i] << "\t";
			 }
			 cout << endl;
    }
		 */  
  }
    
	cout << "COMPUTATION TIME FOR " << num_computations << " ITERATIONS IS " << elapsed << "  (" << elapsed/num_computations << "SECONDS PER ITERATION )" << endl;
	
	cout << "QUITTING SESCTHERM" << endl;
}
