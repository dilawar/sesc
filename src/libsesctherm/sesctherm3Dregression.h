//This will perform a nonlinear curve fitting 
class RegressionNonLinear
{
public:
	
	//Input: computation and datapoints for function to fit
	//Output: coefficients for fitting function based upon computation specified
	void static compute(int computation, int32_t num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary);
	void static computeRC_charging_func(double *p, double *hx, int32_t m, int32_t n, void* data);
	void static computeRC_charging(int num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary_);
	void static computeRC_discharging_func(double *p, double *hx, int32_t m, int32_t n, void* data);
	void static computeRC_discharging(int num_datapoints, double* datapoints_x, double* datapoints_y, double* coefficients, sesctherm3Ddatalibrary* datalibrary_);
};

class RegressionLine
{
public:
	
	typedef map<double, double> Points;
	typedef map<double,double>::iterator Points_iter;
	
	
	
	double slope_;
	double yIntercept_;
	double regressionCoefficient_;
	
	
	RegressionLine(Points & points);
	
	const double slope() const;
	const double yIntercept() const;
	const double regressionCoefficient() const;
						
	static double quad_interpolate(Points & points, double x);
};
