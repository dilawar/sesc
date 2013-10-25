
#include "sesctherm3Dinclude.h"
#include "sesctherm3Ddtm.h"

DTM_pid_control::DTM_pid_control(double iGain,
				 double pGain,
				 double dGain) {
  iGain_=iGain;
  pGain_=pGain;
  dGain_=dGain;
}

double DTM_pid_control::UpdatePID(double temp) {
	double pTerm, dTerm, iTerm;
	double error=setPoint_-temp;
	pTerm = pGain_ * error;   
	// calculate the proportional term
	// calculate the integral state with appropriate limiting
	iState_ += error;
	if (iState_ > iMax_) 
		iState_ = iMax_;
	else if (iState_ < iMin_) 
		iState_ =iMin_;
	iTerm = iGain_ * iState_;  // calculate the integral term
	dTerm = dGain_ * (temp - dState_);
	dState_ = temp;
	return pTerm + iTerm - dTerm;
}

/*  FIXME: Still need to write this code
*  Auto-tuning Method Used:
* 
* The closed loop method prescribes the following procedure:
*
* Step 1: Disable any D and I action of the controller (--> pure P-controller)
* Step 2: Make a setpoint step test and observe the response
* Step 3: Repeat the SP test with increased / decreased controller gain until a stable oscillation is achieved. This gain is called the "ultimate gain" Ku.
* Step 4: Read the oscillation period Pu.
* Step 5: Calculate the parameters according to the following formulas:
*
*     PI: Proportional gain = 0.45 * Ku, integral time =Pu / 1.2
*
*     PID: Proportional gain = 0.6 * Ku, integral time =Pu / 2, derivative time = Tu / 8
*/
void DTM_pid_control::auto_tune(){
 	
}
enum { MESH_STATIC, MESH_POWER_DEPENDENT, RANDOM};
/* DTM Strategies:
* 
* 	Weighted Activity-Rate
* 
*  Weighted Temperature
* 
*  Temperature Threshold
* 
*  Predictive 
* 
*/

DTM_data::DTM_data(sesctherm3Ddatalibrary& datalibrary) {
  datalibrary_=&(datalibrary);
}
