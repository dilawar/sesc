#ifndef SESCTHERM3D_DTM_H
#define SESCTHERM3D_DTM_H

class DTM_data;
class DTM_pid_control;

class DTM_pid_control{
public:	
  DTM_pid_control(double iGain,
		  double pGain,
		  double dGain);
  double DTM_pid_control::UpdatePID(double temp);
  void auto_tune();

  double setPoint_;		//desired temperature
  double dState_;      	// Last position input
  double iState_;      	// Integrator state
  double iMax_;
  double iMin_;  	
  // Maximum and minimum allowable integrator state
  double iGain_;    	// integral gain
  double pGain_;    	// proportional gain
  double dGain_;     	// derivative gain
};

class DTM_data {
public:
    DTM_data(sesctherm3Ddatalibrary& datalibrary);
    friend ostream& operator<<(ostream& os, chip_floorplan& tmp_floorplan);
    void get_DTM_specs(ifstream& if_infile);
	void advance_clock();
//	void calc_delay(
//	int 
	double running_time;
	double current_frequency;
	double current_voltage_scaling;
	sesctherm3Ddatalibrary* datalibrary_;
};
#endif
