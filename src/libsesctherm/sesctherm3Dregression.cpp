#include "sesctherm3Dinclude.h"

//Input: computation and datapoints for function to fit
//Output: coefficients for fitting function based upon computation specified
void RegressionNonLinear::compute(int computation, int32_t num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary){
	//set regression options
	datalibrary->regression_opts_[0]=LM_INIT_MU;
	datalibrary->regression_opts_[1]=1E-15;
	datalibrary->regression_opts_[2]=1E-15;
	datalibrary->regression_opts_[3]=1E-20;
	datalibrary->regression_opts_[4]=LM_DIFF_DELTA;
	
	switch(computation){
		case RC_CHARGING:
			RegressionNonLinear::computeRC_charging(num_datapoints, datapoints_x, datapoints_y, coefficients, datalibrary);
			break;
		case RC_DISCHARGING:
			RegressionNonLinear::computeRC_discharging(num_datapoints, datapoints_x, datapoints_y, coefficients, datalibrary);
			break;
		default:
			cerr << "RegressionNonLinear::compute => wrong computation type specified" << endl;
			exit(1);
	}
}




//P[0]==A
//P[1]==RC
//P[2]==D
//data[i]=t
//h[x]=A(1-exp(-t/RC))+D
void  RegressionNonLinear::computeRC_charging_func(double *p, double *hx, int32_t m, int32_t n, void* data){
	for(int i=0;i<n;i++){
		hx[i]=p[0]*(1-exp((-1.0*((double*)(data))[i])/p[1]))+p[2];
	}
	
}
//This will take the datapoints specified and attempt to fit
//A(1-exp(-t/RC))+D
//coefficients[0]==A
//coefficients[1]==RC
//coefficients[2]==D
void RegressionNonLinear::computeRC_charging(int num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary_){
	int num_vars=3;	//three variables
	
	
	//DEBUGGING
	/*
	cerr << "computeRC_charging =>" << endl;
	cerr << "num_datapoints:" << num_datapoints << endl;
	cerr << "datapoints_x:\t";
	for(int i=0;i<num_datapoints;i++)
			cerr << datapoints_x[i] << "\t";
	cerr << endl << "datapoints_y:\t";
	for(int i=0;i<num_datapoints;i++)
		cerr << datapoints_y[i] << "\t";
	*/
	
	dlevmar_dif(RegressionNonLinear::computeRC_charging_func,
				coefficients,
				datapoints_y,
				num_vars,
				num_datapoints, 
				datalibrary_->config_data_->regression_num_iterations_, 
				datalibrary_->regression_opts_,
				datalibrary_->regression_info_,
				NULL,
				NULL,
				datapoints_x);
	/*
	cerr << "A:" << coefficients[0] << endl;
	cerr << "RC:" << coefficients[1] << endl;
	cerr << "D:" << coefficients[2] << endl;
	*/
}

//P[0]==A
//P[1]==RC
//P[2]==D
//data[i]=t
//h[x]=A(exp(-t/RC))+D
void RegressionNonLinear::computeRC_discharging_func(double *p, double *hx, int32_t m, int32_t n, void* data){
	for(int i=0;i<n;i++){
		hx[i]=p[0]*exp((-1.0*((double*)data)[i])/p[1])+p[2];
	}
}		

//This will take the datapoints specified and attempt to fit
//A(exp(-t/RC))+D
//coefficients[0]==A
//coefficients[1]==RC
//coefficients[2]==D
void RegressionNonLinear::computeRC_discharging(int num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary_){
	
	
	//DEBUGGING
	/*
	cerr << "computeRC_discharging =>" << endl;
	cerr << "num_datapoints:" << num_datapoints << endl;
	cerr << "datapoints_x:\t";
	for(int i=0;i<num_datapoints;i++)
		cerr << datapoints_x[i] << "\t";
	cerr << endl << "datapoints_y:\t";
	for(int i=0;i<num_datapoints;i++)
		cerr << datapoints_y[i] << "\t";
	cerr << endl << "coefficients:\t";
	for(int i=0;i<num_datapoints;i++)
		cerr << coefficients[i] << "\t";
	*/
	int num_vars=3;	//three variables
	dlevmar_dif(RegressionNonLinear::computeRC_discharging_func,
				coefficients,
				datapoints_y,
				num_vars,
				num_datapoints, 
				datalibrary_->config_data_->regression_num_iterations_, 
				datalibrary_->regression_opts_,
				datalibrary_->regression_info_,
				NULL,
				NULL,
				datapoints_x);
	/*
	cerr << "A:" << coefficients[0] << endl;
	cerr << "RC:" << coefficients[1] << endl;
	cerr << "D:" << coefficients[2] << endl;
	*/
}






RegressionLine::RegressionLine(Points & points)
{
	int n = points.size();
	if (n < 2)
		throw (string("Must have at least two points"));
	
	
	double sumx=0,sumy=0,sumx2=0,sumy2=0,sumxy=0;
	double sxx,syy,sxy;
	
	// Conpute some things we need 
	map<double, double>::const_iterator i;
	for (i = points.begin(); i != points.end(); i++)
	{
		double x = i->first;
		double y = i->second;
		
		sumx += x;
		sumy += y;
		sumx2 += (x * x);
		sumy2 += (y * y);
		sumxy += (x * y);
	}
	sxx = sumx2 - (sumx * sumx / n);
	syy = sumy2 - (sumy * sumy / n);
	sxy = sumxy - (sumx * sumy / n);
	
	// Infinite slope_, non existant yIntercept
	if (abs(sxx) == 0)
		throw (string("Inifinite Slope"));
	
	// Calculate the slope_ and yIntercept
	slope_ = sxy / sxx;
	yIntercept_ = sumy / n - slope_ * sumx / n;
	
	// Compute the regression coefficient
	if (abs(syy) == 0)
		regressionCoefficient_ = 1;
	else
		regressionCoefficient_ = sxy / sqrt(sxx * syy);
}


const double RegressionLine::slope() const
{
	return slope_;
}

const double RegressionLine::yIntercept() const
{
	return yIntercept_;
}

const double RegressionLine::regressionCoefficient() const
{
	return regressionCoefficient_;
}



/****************************************************
*     Polynomial Interpolation or Extrapolation        *
*            of a Discrete Function                    *
* -------------------------------------------------    *
* INPUTS:                                              *
*   (iter)->first:   (xcoords)                       *
*   (iter)->second:   (ycoords)                      *
*    X:    Interpolation abscissa value                *
* OUTPUTS:                                             *
*    y_estimate:Returned estimation of function for X  *
****************************************************/					
double RegressionLine::quad_interpolate(Points & points, double x){
	std::vector<double> c,d;
	long double den,dif,dift,ho,hp,w;
	long double y_estimate,y_estimate_error;
	
	std::vector<double> xa;
	std::vector<double> ya;
	
	Points_iter piter=points.begin();
	int iterator=1;
	xa.push_back(0);
	for (piter=points.begin(); piter!=points.end(); piter++) {
		xa.push_back(piter->first);
		ya.push_back(piter->second);
	}
	
	
	y_estimate=0;
	y_estimate_error=0;
	int ns=1;
	dif = fabs(x - xa[1]);
	int i=1;
	int n=(int)xa.size()-1;
	for (i=1;i<=n;i++) {
		dift = fabs(x - xa[i]);
		if (LT(dift,dif)) {
			ns=i;                     //index of closest table entry
			dif = dift;
		}
		
		c.push_back(ya[i]);           //Initialize the C's and D's
		d.push_back(ya[i]);
	}
	
	y_estimate=ya[ns];         //Initial approximation of Y
	return(y_estimate);
	ns--;
	
	//FIXME: the following  algorithm return invalid data
	for(int m=1;m<n;m++){
		for(int i=1;i<=n-m;i++){
			ho=xa[i]-x;
			hp=xa[i+m]-x;
			w=c[i+1]-d[i];
			den=ho-hp;
			if (EQ(den,0.0)) {
				return(y_estimate);
			}
			den=w/den;
			d[i]=hp*den;         //Update the C's and D's
			c[i]=ho*den;
		}
		if (2*ns < n-m)        //After each column in the tableau xa is completed,
			y_estimate_error=c[ns+1];         //we decide which correction, C or D, we want to   
		else {                 //add to our accumulating value of Y, i.e. which   
			y_estimate_error=d[ns];           //path to take through the tableau, for (king up or
			ns--;                //down. We do this in such a way as to take the    
				}                      //most "straight line" route through the tableau to
		y_estimate = y_estimate + y_estimate_error;         //its apex, updating NS accordingly to keep track  
															//of where we are. This route keeps the partial    
															//approximations centered (insofar as possible) on 
			}						   //the target X.The last DY added is thus the error 
	
	return(y_estimate);
		}

