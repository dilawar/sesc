#include "sesctherm3Dinclude.h"


//MODEL_UNIT
void model_unit::zero(model_unit& model_unit, sesctherm3Ddatalibrary* datalibrary){
	if(datalibrary == NULL){
		cerr << "FATAL: model_unit::model_unit::zero called, but datalibrary pointer was NULL!" << endl;
	}
	model_unit.datalibrary_=datalibrary;
	//model_unit.not_hot_spot_=true;
	//model_unit.hot_spot_duration_=0;
	//model_unit.hot_spot_count_=0;
	model_unit.defined_=false;
	model_unit.source_ucool_flp_=NULL; //this is a reference to the source ucooler flp that this modelunit is a part of
	model_unit.source_chip_flp_=NULL; //this is a reference to the source chip flp that this modelunit is a part of
	model_unit.defined_=false;          //is this modelunit used?
	//model_unit.name_="";            //name of region that unit belongs to
	model_unit.flp_percentage_=0;      //percentage of the region's area that the unit belongs to. Note: this does need to be used when calculating energy data
									   //as we are using energy/per unit area anyway. However, it does need to be used when calculating an average temperature
									   //per flp
	model_unit.ucool_percentage_=0;   //percentage of the microcooler that the unit belongs to. This is used to calculate the proper resistances and internal
									  //heat generation for each of the micro-coolers
	model_unit.ucool_rth_=0;          //this is Rth/2
									  //solution data
	model_unit.temperature_=NULL;
	model_unit.previous_temperature_=NULL; //this was the previously calculated result (initially the steady-temp value)
										  //gamma
	model_unit.temperature_index_=0;
		
	model_unit.gamma_=0;
	//row
	model_unit.row_=0;
	//specific heat (Cp) or (C)
	model_unit.specific_heat_=0;
	
	//material conductances
	//m,n,o+1 (up)
	model_unit.conduct_up_=0;
	//m+1,n,o (right)
	model_unit.conduct_right_=0;
	//m,n,o-1 (down)
	model_unit.conduct_down_=0;
	//m-1,n,o (left)
	model_unit.conduct_left_=0;
	//m,n-1,o (bottom)
	model_unit.conduct_bottom_=0;
	//m,n+1,o (top)
	model_unit.conduct_top_=0;
	//m,n,o (center)
	model_unit.conduct_center_left_=0;
	model_unit.conduct_center_right_=0;
	model_unit.conduct_center_up_=0;
	model_unit.conduct_center_down_=0;
	model_unit.conduct_center_top_=0;
	model_unit.conduct_center_bottom_=0;
	
	//interface resistances
	//m,n,o+1 (up)
	model_unit.resist_up_=0;
	//m+1,n,o (right)
	model_unit.resist_right_=0;
	//m,n,o-1 (down)
	model_unit.resist_down_=0;
	//m-1,n,o (left)
	model_unit.resist_left_=0;
	//m,n-1,o (bottom)
	model_unit.resist_bottom_=0;
	//m,n+1,o (top)
	model_unit.resist_top_=0;
	
	//dimensions
	model_unit.x1_=0;
	model_unit.x2_=0;
	model_unit.x3_=0;
	model_unit.y1_=0;
	model_unit.y2_=0;
	model_unit.y3_=0;
	model_unit.z1_=0;
	model_unit.z2_=0;
	model_unit.z3_=0;
	
	//the governing equation for the given model unit
	model_unit.governing_equation_=0;
	
	//the value for each of the terms for the governing equation
	// Tm,n,o
	model_unit.t_mno=NULL;
	// Tm-1,n,o
	model_unit.t_m_M1_no=NULL;
	// Tm+1,n,o
	model_unit.t_m_P1_no=NULL;
	// Tm,n-1,o
	model_unit.t_mn_M1_o=NULL;
	// Tm,n+1,o
	model_unit.t_mn_P1_o=NULL;
	// Tm,n,o-1
	model_unit.t_mno_M1=NULL;
	// Tm,n,o+1
	model_unit.t_mno_P1=NULL;
	// T_inf m,n,o
	model_unit.tINF_mno=NULL;
	// T_inf m-1,n,o
	model_unit.tINF_m_M1_no=NULL;
	// T_inf m+1,n,o
	model_unit.tINF_m_P1_no=NULL;
	// T_inf m,n-1,o
	model_unit.tINF_mn_M1_o=NULL;
	// T_inf m,n+1,o
	model_unit.tINF_mn_P1_o=NULL;
	// T_inf m,n,o-1
	model_unit.tINF_mno_M1=NULL;
	// T_inf m,n,o+1
	model_unit.tINF_mno_P1=NULL;
	// Tvil,m,o
	model_unit.tVIL_mo=NULL;
	
	model_unit.power_per_unit_area_=-1.0;
}

model_unit::model_unit(sesctherm3Ddatalibrary* datalibrary) {
	if(datalibrary == NULL){
		cerr << "FATAL: model_unit::model_unit constructor called, but datalibrary pointer was NULL!" << endl;
	}
	datalibrary_=datalibrary;
	//not_hot_spot_=true;
	//hot_spot_duration_=0;
	//hot_spot_count_=0;
	defined_=false;
	source_ucool_flp_=NULL; //this is a reference to the source ucooler flp that this modelunit is a part of
	source_chip_flp_=NULL; //this is a reference to the source chip flp that this modelunit is a part of
	defined_=false;          //is this modelunit used?
	//name_="";            //name of region that unit belongs to
	flp_percentage_=0;      //percentage of the region's area that the unit belongs to. Note: this does need to be used when calculating energy data
							//as we are using energy/per unit area anyway. However, it does need to be used when calculating an average temperature
							//per flp
	ucool_percentage_=0;   //percentage of the microcooler that the unit belongs to. This is used to calculate the proper resistances and internal
						   //heat generation for each of the micro-coolers
	ucool_rth_=0;          //this is Rth/2
						   //solution data
	temperature_=NULL;
	previous_temperature_=NULL; //this was the previously calculated result (initially the steady-temp value)
							   //gamma
	gamma_=0;
	//row
	row_=0;
	
	//specific heat (Cp) or (C)
	specific_heat_=0;
	
	//material conductances
	//m,n,o+1 (up)
	conduct_up_=0;
	//m+1,n,o (right)
	conduct_right_=0;
	//m,n,o-1 (down)
	conduct_down_=0;
	//m-1,n,o (left)
	conduct_left_=0;
	//m,n-1,o (bottom)
	conduct_bottom_=0;
	//m,n+1,o (top)
	conduct_top_=0;
	//m,n,o (center)
	conduct_center_left_=0;
	conduct_center_right_=0;
	conduct_center_up_=0;
	conduct_center_down_=0;
	conduct_center_top_=0;
	conduct_center_bottom_=0;
	
	//interface resistances
	//m,n,o+1 (up)
	resist_up_=0;
	//m+1,n,o (right)
	resist_right_=0;
	//m,n,o-1 (down)
	resist_down_=0;
	//m-1,n,o (left)
	resist_left_=0;
	//m,n-1,o (bottom)
	resist_bottom_=0;
	//m,n+1,o (top)
	resist_top_=0;
	
	//dimensions
	x1_=0;
	x2_=0;
	x3_=0;
	y1_=0;
	y2_=0;
	y3_=0;
	z1_=0;
	z2_=0;
	z3_=0;
	
	//the governing equation for the given model unit
	governing_equation_=0;
	
	//the value for each of the terms for the governing equation
	// Tm,n,o
	t_mno=NULL;
	// Tm-1,n,o
	t_m_M1_no=NULL;
	// Tm+1,n,o
	t_m_P1_no=NULL;
	// Tm,n-1,o
	t_mn_M1_o=NULL;
	// Tm,n+1,o
	t_mn_P1_o=NULL;
	// Tm,n,o-1
	t_mno_M1=NULL;
	// Tm,n,o+1
	t_mno_P1=NULL;
	// T_inf m,n,o
	tINF_mno=NULL;
	// T_inf m-1,n,o
	tINF_m_M1_no=NULL;
	// T_inf m+1,n,o
	tINF_m_P1_no=NULL;
	// T_inf m,n-1,o
	tINF_mn_M1_o=NULL;
	// T_inf m,n+1,o
	tINF_mn_P1_o=NULL;
	// T_inf m,n,o-1
	tINF_mno_M1=NULL;
	// T_inf m,n,o+1
	tINF_mno_P1=NULL;
	// Tvil,m,o
	tVIL_mo=NULL;
	
	power_per_unit_area_=-1.0;
}


model_unit::model_unit(sesctherm3Ddatalibrary *datalibrary,
					   double conduct_up,
                       double conduct_right,
                       double conduct_down,
                       double conduct_left,
                       double conduct_bottom,
                       double conduct_top,
					   double conduct_center_up,
                       double conduct_center_right,
                       double conduct_center_down,
                       double conduct_center_left,
                       double conduct_center_bottom,
                       double conduct_center_top,
                       double resist_up,
                       double resist_right,
                       double resist_down,
                       double resist_bottom,
                       double resist_top,
                       double x1,double x2,double  x3,
                       double y1,double y2,double  y3,
                       double z1,double z2,double  z3,
                       double governing_equation) {
	
    conduct_up_    = conduct_up;
    conduct_right_ = conduct_right;
    conduct_down_  = conduct_down;
    conduct_left_  = conduct_left;
    conduct_bottom_= conduct_bottom;
    conduct_top_   = conduct_top;
	
    conduct_center_up_    = conduct_center_up;
    conduct_center_right_ = conduct_center_right;
    conduct_center_down_  = conduct_center_down;
    conduct_center_left_  = conduct_center_left;
    conduct_center_bottom_= conduct_center_bottom;
    conduct_center_top_   = conduct_center_top;
	
    resist_up_     = resist_up;
    resist_right_  = resist_right;
    resist_down_   = resist_down;
    resist_bottom_ = resist_bottom;
    resist_top_    = resist_top;
	
    x1_ = x1;
    x2_ = x2;
    x3_ = x3;
	
    y1_ = y1;
    y2_ = y2;
    y3_ = y3;
	
    z1_ = z1;
    z2_ = z2;
    z3_ = z3;
    defined_=true;
    governing_equation_=governing_equation;
    source_chip_flp_=NULL;
    source_ucool_flp_=NULL;
	power_per_unit_area_=-1.0;
}

//set power per unit area (W/m^3)
void model_unit::set_power(double power){
	power_per_unit_area_=power/(y2_*source_chip_flp_->width_*source_chip_flp_->height_);
	
}


SUElement_t* model_unit::get_temperature(){
  return(*temperature_+temperature_index_);
}

SUElement_t* model_unit::get_previous_temperature(){
  return(*previous_temperature_+temperature_index_);	
}


void model_unit::accumulate_rc(){
	datapoints_x_.push_back(datalibrary_->time_);
	datapoints_y_.push_back(*get_temperature());
}


void model_unit::compute_rc(){
	
	double* datapoints_x=new double[datapoints_x_.size()];
	double* datapoints_y=new double[datapoints_y_.size()];
	
	//make sure that the timestep starts at zero (offset by the first time datapoint)
	for(uint32_t i=0; i<datapoints_x_.size();i++){
		datapoints_x[i]=datapoints_x_[i]-datapoints_x_[0];
		datapoints_y[i]=datapoints_y_[i];
	}
	
	double* coefficients=new double[3];			//coefficients[0]==A
												//coefficients[2]==D
	//the temperature has been increasing, fit to charging equation: A*(1-exp(-1.0*t/RC))+D
	if(datapoints_y_[datapoints_y_.size()-1] > datapoints_y_[0])
		RegressionNonLinear::compute(RC_CHARGING,
					     datapoints_x_.size(),
					     datapoints_x,
					     datapoints_y,
					     coefficients,
					     datalibrary_);
	//the temperature has been decreasing, fit to the discharging equation: A*(exp(-1.0*t/RC))+D
	else if(datapoints_y_[datapoints_y_.size()-1] < datapoints_y_[0])
		RegressionNonLinear::compute(RC_DISCHARGING,
					     datapoints_x_.size(),
					     datapoints_x,
					     datapoints_y,
					     coefficients,
					     datalibrary_);	
	//otherwise the temperature is constant, this means that RC is effectively infinity,
	//as A*(exp(-1.0*t/inf))+D = A*exp(0)+D = A+D = constant 
	else
		coefficients[1]=pow(10.0,10.0);
	
	
	double tau=coefficients[1]; //time constant == coefficients[1]
	//now determine what the effective resistances and specific heat will be
	//we do this by assuming that the resistances and specific heat will be in the same proportions as the current R's and C
	
	//compute SUM(Ri,i=1..n)
	double c_tot = specific_heat_*row_*x2_*y2_*z2_;
	
	double r_tot = 0;
	
	if(t_m_M1_no!=0)
		r_tot+=*t_m_M1_no;
	
	if(t_m_P1_no!=0)
		r_tot+=*t_m_P1_no;
	
	if(t_mn_M1_o!=0)
		r_tot+=*t_mn_M1_o;
	
	if(t_mn_P1_o!=0)
		r_tot+=*t_mn_P1_o;
	
	if(t_mno_M1!=0)
		r_tot+=*t_mno_M1;
	
	if(t_mno_P1!=0)
		r_tot+=*t_mno_P1;
	
	if(tINF_m_M1_no!=0)
		r_tot+=*tINF_m_M1_no;
	
	if(tINF_m_P1_no!=0)
		r_tot+=*tINF_m_P1_no;
	
	if(tINF_mn_M1_o!=0)
		r_tot+=*tINF_mn_M1_o;

	if(tINF_mn_P1_o!=0)
		r_tot+=*tINF_mn_P1_o;

	if(tINF_mno_M1!=0)
		r_tot+=*tINF_mno_M1;
	
	if(tINF_mno_P1!=0)
		r_tot+=*tINF_mno_P1;
	
	double c_tot_percent =		c_tot / (r_tot + c_tot);
	double r_tot_percent =		r_tot / (r_tot + c_tot);
	double r_m_M1_no_percent =	0;
	double r_m_P1_no_percent =	0;
	double r_mn_M1_o_percent =	0;
	double r_mn_P1_o_percent =	0;
	double r_mno_M1_percent =	0;
	double r_mno_P1_percent =	0;
	double rINF_m_M1_no_percent = 0;
	double rINF_m_P1_no_percent = 0;
	double rINF_mn_M1_o_percent = 0;
	double rINF_mn_P1_o_percent = 0;
	double rINF_mno_M1_percent = 0;
	double rINF_mno_P1_percent = 0;
	
	if(t_m_M1_no!=0)
		r_m_M1_no_percent =	(*t_m_M1_no/r_tot)*r_tot_percent;
	
	if(t_m_P1_no!=0)
		r_m_P1_no_percent =	(*t_m_P1_no/r_tot)*r_tot_percent;
	
	if(t_mn_M1_o!=0)
		r_mn_M1_o_percent =	(*t_mn_M1_o/r_tot)*r_tot_percent;
	
	if(t_mn_P1_o!=0)
		r_mn_P1_o_percent =	(*t_mn_P1_o/r_tot)*r_tot_percent;
	
	if(t_mno_M1!=0)
		r_mno_M1_percent =	(*t_mno_M1/r_tot)*r_tot_percent;
	
	if(t_mno_P1!=0)
		 r_mno_P1_percent =	(*t_mno_P1/r_tot)*r_tot_percent;
	
	if(tINF_m_M1_no!=0)
		 rINF_m_M1_no_percent = (*tINF_m_M1_no/r_tot)*r_tot_percent;
	
	if(tINF_m_P1_no!=0)
		rINF_m_P1_no_percent = (*tINF_m_P1_no/r_tot)*r_tot_percent;
	
	if(tINF_mn_M1_o!=0)
		rINF_mn_M1_o_percent = (*tINF_mn_M1_o/r_tot)*r_tot_percent;
	
	if(tINF_mn_P1_o!=0)
		rINF_mn_P1_o_percent = (*tINF_mn_P1_o/r_tot)*r_tot_percent;
	
	if(tINF_mno_M1!=0)
		rINF_mno_M1_percent = (*tINF_mno_M1/r_tot)*r_tot_percent;
	
	if(tINF_mno_P1!=0)
		rINF_mno_P1_percent = (*tINF_mno_P1/r_tot)*r_tot_percent;
	

		
	//Solve two equations:	1) R=r_tot_percent(R+C)
	//						2) RC=tau
	//Closed-form solution: R=sqrt[(tau*r_tot_percent)/(1-r_tot_percent)]
	double r_tot_eff = sqrt((tau*r_tot_percent)/(1-r_tot_percent));
	
	//now scale each of the resistances assuming that the resistances maintain the same
	//relative proportions
	t_m_M1_no_eff=r_m_M1_no_percent*r_tot_eff;
	t_m_P1_no_eff=r_m_P1_no_percent*r_tot_eff;
	t_mn_M1_o_eff=r_mn_M1_o_percent*r_tot_eff;
	t_mn_P1_o_eff=r_mn_P1_o_percent*r_tot_eff;
	t_mno_M1_eff=r_mno_M1_percent*r_tot_eff;
	t_mno_P1_eff=r_mno_P1_percent*r_tot_eff;
	tINF_m_M1_no_eff=rINF_m_M1_no_percent*r_tot_eff;
	tINF_m_P1_no_eff=rINF_m_P1_no_percent*r_tot_eff;
	tINF_mn_M1_o_eff=rINF_mn_M1_o_percent*r_tot_eff;
	tINF_mn_P1_o_eff=rINF_mn_P1_o_percent*r_tot_eff;
	tINF_mno_M1_eff=rINF_mno_M1_percent*r_tot_eff;
	tINF_mno_P1_eff=rINF_mno_P1_percent*r_tot_eff;
	
	double c_tot_eff = tau/r_tot_eff;
	//assume that volume and density stays constant (only specific heat changes)
	//scale the specific heat by the percent increase in the heat capacitance
	//(c_tot_eff/c_tot)
	specific_heat_eff_=(c_tot_eff/(c_tot))*specific_heat_;
	
#ifdef _SESCTHERM_DEBUG
	//check to make sure that the computation is correct
	if(!EQ(r_tot_eff, r_tot_percent*(r_tot_eff+c_tot_eff))){
		cerr << "model_unit::compute_rc => error in r_tot_eff computation!" << endl;
		exit(1);
	}

#endif
}

//calulate all the term values given the equation number
//time_increment is the time step
//calc_solution_only is used if we don't need to recompute the matrix 
//(recomputation is necessary when timestep changes, material properties change, or model geometry changes)
//accumulate_rc is used when we need to accumuate the [time,temperature data] for each timestep
//use_rc is used when we want to use an effective RC time constant to fast-forward the model
void model_unit::calculate_terms(double time_increment, bool calc_solution_only, bool use_rc){
	
	
    if (!defined_) { //if the model unit is unused, set everything to zero
        t_mno=NULL;
		
        //calculate  Tm-1,n,o
        t_m_M1_no=NULL;
        //calculate Tm+1,n,o
        t_m_P1_no=NULL;
        //calculate Tm,n-1,o
        t_mn_M1_o=NULL;
        //calculate Tm,n+1,o
        t_mn_P1_o=NULL;
        //calculate Tm,n,o-1
        t_mno_M1=NULL;
        //calculate Tm,n,o+1
        t_mno_P1=NULL;
		
		
		
        //calculate T_inf m,n,o
        tINF_mno=NULL;
		
        //calculate T_inf m-1,n,o
		
        tINF_m_M1_no=NULL;
		
        //calculate T_inf m+1,n,o
		
        tINF_m_P1_no=NULL;
		
        //calculate T_inf m,n-1,o
		
        tINF_mn_M1_o=NULL;
		
        //calculate T_inf m,n+1,o
		
        tINF_mn_P1_o=NULL;
		
        //calculate T_inf m,n,o-1
		
        tINF_mno_M1=NULL;
		
        //calculate T_inf m,n,o+1
		
        tINF_mno_P1=NULL;
		
        //calculate Tvil,m,o
		
        tVIL_mo=NULL;
        //calculate solution data
		
        temperature_=NULL;
		previous_temperature_=NULL;
		temperature_index_=0;
		sample_temperature_=0;
        return;
    }
	
	if(temperature_==NULL){
		cerr << "FATAL: sesctherm3Dbase::calculate_terms() => temperature_ pointer is undefined." << endl;
		exit(1);
	}
	
	if(previous_temperature_==NULL){
		cerr << "FATAL: sesctherm3Dbase::calculate_terms() => previous_temperature_ pointer is undefined." << endl;
		exit(1);
	}		
	
	
	//compute the convection_coefficient
	/*
    double convection_coefficient;
    if(datalibrary_->config_data_->fan_used_)
		//convection_coefficient=convection_coefficient_layer_;
		convection_coefficient_=3.886*sqrt(datalibrary_->config_data_->fan_velocity_/x2_);
    else if(datalibrary_->config_data_->oil_used_)
		convection_coefficient_=datalibrary_->config_data_->oil_layer_convection_coefficient_;
    else
		//considered to be .5m/s ambient air travel in case from fan etc
		convection_coefficient_=3.886*sqrt(.5/x2_);
	*/
	
    //calc_hot_spot_count();
    //calc_hot_spot_duration();
    calc_gamma(time_increment, use_rc);
	
	

	
	//if this is heat transfer to another model unit though HEAT_CONVECTION_TRANSFER and/or HEAT_CONDUCTION_TRANSFER and/or HEAT_RADIATION_TRANSFER
	if(governing_equation_==BASIC_EQUATION)
	{
		//if temperature locking is enabled, then the equation is T_mno=locktemp
		if(temp_locking_enabled_) {
			if(!calc_solution_only) {
				I(t_mno!=NULL);
				
				if(t_mno!=NULL)
					*t_mno=1;
				
				//calculate  Tm-1,n,o
				if(t_m_M1_no!=NULL)
					*t_m_M1_no=0;
				
				//calculate Tm+1,n,o
				if(t_m_P1_no!=NULL)
					*t_m_P1_no=0;
				//calculate Tm,n-1,o
				if(t_mn_M1_o!=NULL)
					*t_mn_M1_o=0;
				//calculate Tm,n+1,o
				if(t_mn_P1_o!=NULL)
					*t_mn_P1_o=0;
				//calculate Tm,n,o-1
				if(t_mno_M1!=NULL)
					*t_mno_M1=0;
				//calculate Tm,n,o+1
				if(t_mno_P1!=NULL)
					*t_mno_P1=0;
				
				//calculate T_inf m,n,o
				if(tINF_mno!=NULL)
					*tINF_mno=0;
				
				//calculate T_inf m-1,n,o
				if(tINF_m_M1_no!=NULL)
					*tINF_m_M1_no=0;
				
				//calculate T_inf m+1,n,o
				if(tINF_m_P1_no!=NULL)
					*tINF_m_P1_no=0;
				
				//calculate T_inf m,n-1,o
				if(tINF_mn_M1_o!=NULL)
					*tINF_mn_M1_o=0;
				
				//calculate T_inf m,n+1,o
				if(tINF_mn_P1_o!=NULL)
					*tINF_mn_P1_o=0;
				
				//calculate T_inf m,n,o-1
				if(tINF_mno_M1!=NULL)
					*tINF_mno_M1=0;
				
				//calculate T_inf m,n,o+1
				if(tINF_mno_P1!=NULL)
					*tINF_mno_P1=0;
				
				//calculate Tvil,m,o
				if(tVIL_mo!=NULL)
					*tVIL_mo=0;
			}
			*get_temperature()=lock_temp_;
			*get_previous_temperature()=lock_temp_;
			
			return;
		
		}
		else if (!calc_solution_only) {
			I(t_mno!=NULL);
			
			if(t_mno!=NULL)
				*t_mno=gamma_;
			
			//calculate  Tm-1,n,o
			if(t_m_M1_no!=NULL)
				*t_m_M1_no=0;
			
			//calculate Tm+1,n,o
			if(t_m_P1_no!=NULL)
				*t_m_P1_no=0;
			//calculate Tm,n-1,o
			if(t_mn_M1_o!=NULL)
				*t_mn_M1_o=0;
			//calculate Tm,n+1,o
			if(t_mn_P1_o!=NULL)
				*t_mn_P1_o=0;
			//calculate Tm,n,o-1
			if(t_mno_M1!=NULL)
				*t_mno_M1=0;
			//calculate Tm,n,o+1
			if(t_mno_P1!=NULL)
				*t_mno_P1=0;
			
	        //calculate T_inf m,n,o
			if(tINF_mno!=NULL)
				*tINF_mno=0;
			
			//calculate T_inf m-1,n,o
			if(tINF_m_M1_no!=NULL)
				*tINF_m_M1_no=0;
			
			//calculate T_inf m+1,n,o
			if(tINF_m_P1_no!=NULL)
				*tINF_m_P1_no=0;
			
			//calculate T_inf m,n-1,o
			if(tINF_mn_M1_o!=NULL)
				*tINF_mn_M1_o=0;
			
			//calculate T_inf m,n+1,o
			if(tINF_mn_P1_o!=NULL)
				*tINF_mn_P1_o=0;
			
			//calculate T_inf m,n,o-1
			if(tINF_mno_M1!=NULL)
				*tINF_mno_M1=0;
			
			//calculate T_inf m,n,o+1
			if(tINF_mno_P1!=NULL)
				*tINF_mno_P1=0;
			
			//calculate Tvil,m,o
			if(tVIL_mo!=NULL)
				*tVIL_mo=0;
			

			
						
			double conduct_convection_left=0;
			double conduct_convection_right=0;
			double conduct_convection_up=0;
			double conduct_convection_down=0;
			double conduct_convection_top=0;
			double conduct_convection_bottom=0;
				
			double conduct_conduction_left=0;
			double conduct_conduction_right=0;
			double conduct_conduction_up=0;
			double conduct_conduction_down=0;
			double conduct_conduction_top=0;
			double conduct_conduction_bottom=0;			

			double conduct_radiation_left=0;
			double conduct_radiation_right=0;
			double conduct_radiation_up=0;
			double conduct_radiation_down=0;
			double conduct_radiation_top=0;
			double conduct_radiation_bottom=0;
			
			
			double radiation_heat_transfer_coefficient_left=0;
			double radiation_heat_transfer_coefficient_right=0;
			double radiation_heat_transfer_coefficient_top=0;
			double radiation_heat_transfer_coefficient_bottom=0;
			double radiation_heat_transfer_coefficient_up=0;
			double radiation_heat_transfer_coefficient_down=0;
				
			double area_left_right=z2_*y2_;
			double area_down_up=x2_*y2_;
			double area_bottom_top=x2_*z2_;
			
			double temp_left=0;
			double temp_right=0;
			double temp_up=0;
			double temp_down=0;
			double temp_bottom=0;
			double temp_top=0;
			
			if(model_left_!=0)
				temp_left=*model_left_->get_previous_temperature();
			if(model_right_!=0)
				temp_right=*model_right_->get_previous_temperature();
			if(model_down_!=0)
				temp_down=*model_down_->get_previous_temperature();
			if(model_up_!=0)
				temp_up=*model_up_->get_previous_temperature();
			if(model_bottom_!=0)
				temp_bottom=*model_bottom_->get_previous_temperature();
			if(model_top_!=0)
				temp_top=*model_top_->get_previous_temperature();

			//set conductances for heat transfer to left model unit
			if(heat_transfer_methods_left_&(1<<HEAT_CONVECTION_TRANSFER))
				conduct_convection_left=(convection_coefficient_*area_left_right);
			if(heat_transfer_methods_left_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_left=1/((x2_/2)/(conduct_center_left_*area_left_right) + resist_left_ +  (x1_/2)/(conduct_left_*area_left_right));
			if(heat_transfer_methods_left_&(1<<HEAT_RADIATION_TRANSFER)){
					radiation_heat_transfer_coefficient_left=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(z2_,y2_,x1_/2+x2_/2)*pow((*get_previous_temperature()+273.15)*(temp_left+273.15),3/2); 
					conduct_radiation_left=(radiation_heat_transfer_coefficient_left*area_left_right);
			}

			//set conductances for heat transfer to right model unit
			if(heat_transfer_methods_right_&(1<<HEAT_CONVECTION_TRANSFER)){
			
				conduct_convection_right=(convection_coefficient_*area_left_right);		
				
			}
			if(heat_transfer_methods_right_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_right=1/((x2_/2)/(conduct_center_right_*area_left_right) + resist_right_ +  (x3_/2)/(conduct_right_*area_left_right));		
			if(heat_transfer_methods_right_&(1<<HEAT_RADIATION_TRANSFER)){
				radiation_heat_transfer_coefficient_right=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(z2_,y2_,x3_/2+x2_/2)*pow((*get_previous_temperature()+273.15)*(temp_right+273.15),3/2); 
				conduct_radiation_right=(radiation_heat_transfer_coefficient_right*area_left_right);
			}

			//set conductances for heat transfer to bottom model unit
			if(heat_transfer_methods_bottom_&(1<<HEAT_CONVECTION_TRANSFER)) 
				conduct_convection_bottom=(convection_coefficient_*area_bottom_top);
			if(heat_transfer_methods_bottom_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_bottom=1/((y2_/2)/(conduct_center_bottom_*area_bottom_top) + resist_bottom_ + (y1_/2)/(conduct_bottom_*area_bottom_top));
			if(heat_transfer_methods_bottom_&(1<<HEAT_RADIATION_TRANSFER)){
				radiation_heat_transfer_coefficient_bottom=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(x2_,z2_,y1_/2+y2_/2)*pow((*get_previous_temperature()+273.15)*(temp_bottom+273.15),3/2); 
				conduct_radiation_bottom=(radiation_heat_transfer_coefficient_bottom*area_bottom_top);
			}			

			//set conductances for heat transfer to top model unit
			if(heat_transfer_methods_top_&(1<<HEAT_CONVECTION_TRANSFER)) 
				conduct_convection_top=(convection_coefficient_*area_bottom_top);
			if(heat_transfer_methods_top_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_top=1/((y2_/2)/(conduct_center_top_*area_bottom_top) + resist_top_ + (y3_/2)/(conduct_top_*area_bottom_top));
			if(heat_transfer_methods_top_&(1<<HEAT_RADIATION_TRANSFER)){
				radiation_heat_transfer_coefficient_top=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(x2_,z2_,y3_/2+y2_/2)*pow((*get_previous_temperature()+273.15)*(temp_top+273.15),3/2); 
				conduct_radiation_top=(radiation_heat_transfer_coefficient_top*area_bottom_top);
			}			
			
			//set conductances for heat transfer to down model unit
			if(heat_transfer_methods_down_&(1<<HEAT_CONVECTION_TRANSFER)) 
				conduct_convection_down=(convection_coefficient_*area_down_up);
			if(heat_transfer_methods_down_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_down=1/((z2_/2)/(conduct_center_down_*area_down_up) + resist_down_ +  (z1_/2)/(conduct_down_*area_down_up));
			if(heat_transfer_methods_down_&(1<<HEAT_RADIATION_TRANSFER)){
				radiation_heat_transfer_coefficient_down=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(x2_,y2_,z1_/2+z2_/2)*pow((*get_previous_temperature()+273.15)*(temp_down+273.15),3/2); 
				conduct_radiation_down=(radiation_heat_transfer_coefficient_down*area_down_up);
			}			
			
			//set conductances for heat transfer to up model unit
			if(heat_transfer_methods_up_&(1<<HEAT_CONVECTION_TRANSFER)) 
				conduct_convection_up=(convection_coefficient_*area_down_up);
			if(heat_transfer_methods_up_&(1<<HEAT_CONDUCTION_TRANSFER)) 
				conduct_conduction_up=1/((z2_/2)/(conduct_center_up_*area_down_up) + resist_up_ +  (z3_/2)/(conduct_up_*area_down_up));
			if(heat_transfer_methods_up_&(1<<HEAT_RADIATION_TRANSFER)){
				radiation_heat_transfer_coefficient_up=4*emissivity_*BOLTZMAN_CONST*calc_view_factor(x2_,y2_,z3_/2+z2_/2)*pow((*get_previous_temperature()+273.15)*(temp_up+273.15),3/2); 
				conduct_radiation_up=(radiation_heat_transfer_coefficient_up*area_down_up);
			}	
			
			
			//the value for each of the terms for the given governing equation
			//************** Create T_m-1,n,o **************
			if(EQ(x1_,0.0)){
//				*t_mno+=0;
//				*t_m_M1_no=0;
			}
			else if (!EQ(x1_,-1.0))	   //if T_m-1,n,o isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_left+conduct_convection_left+conduct_radiation_left))
							);	
					*t_m_M1_no=-1.0*(
								  (conduct_conduction_left+conduct_convection_left+conduct_radiation_left)
							);	
				}
				else{
					*t_mno+=t_m_M1_no_eff;	
					*t_m_M1_no=t_m_M1_no_eff;
				}
			}
			else				//if T_m-1,n,o is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(y2_*z2_)/
							((x2_/(2*conduct_center_left_)) + 1/(convection_coefficient_/sqrt(y2_*z2_)))
							);
					*tINF_m_M1_no=-1.0*(
									 (y2_*z2_)/
									 ((x2_/(2*conduct_center_left_)) + 1/(convection_coefficient_/sqrt(y2_*z2_)))
									 );
					*tINF_mno+=*tINF_m_M1_no;					
				}
				else{
					*t_mno+=tINF_m_M1_no_eff;
					*tINF_m_M1_no=tINF_m_M1_no_eff;
					*tINF_mno+=*tINF_m_M1_no;
				}
					
			}
			//************** Create T_m+1,n,o **************
			if(EQ(x3_,0.0)){
//				*t_mno+=0;
//				*t_m_P1_no=0;
			}
			else if (!EQ(x3_,-1.0))	   //if T_m+1,n,o isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_right+conduct_convection_right+conduct_radiation_right))
							);	
					*t_m_P1_no=-1.0*(
								  ((conduct_conduction_right+conduct_convection_right+conduct_radiation_right))
								  );	
				}
				else{
					*t_mno+=t_m_P1_no_eff;
					*t_m_P1_no=t_m_P1_no_eff;
				}
			}
			else				//if T_m+1,n,o is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(y2_*z2_)/
							((x2_/(2*conduct_center_right_)) + 1/(convection_coefficient_/sqrt(y2_*z2_)))
							);
					*tINF_m_P1_no=-1.0*(
									 (y2_*z2_)/
									 ((x2_/(2*conduct_center_right_)) + 1/(convection_coefficient_/sqrt(y2_*z2_)))
									 );
					*tINF_mno+=*tINF_m_P1_no;					
				}
				else{
					*t_mno+=tINF_m_P1_no_eff;
					*tINF_m_P1_no=tINF_m_P1_no_eff;
					*tINF_mno+=tINF_m_P1_no_eff;
				}
			}
			//************** Create T_m,n,o-1 **************
			if(EQ(z1_,0.0)){
//				*t_mno+=0;
//				*t_mno_M1=0;
			}
			else if (!EQ(z1_,-1.0))	   //if T_m,n,o-1 isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_down+conduct_convection_down+conduct_radiation_down))
							);	
					*t_mno_M1=-1.0*(
								 ((conduct_conduction_down+conduct_convection_down+conduct_radiation_down))
								 );	
				}
				else{
					*t_mno+=t_mno_M1_eff;
					*t_mno_M1=t_mno_M1_eff;
				}
			}
			else				//if T_m,n,o-1 is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(y2_*z2_)/
							((z2_/(2*conduct_center_down_)) + 1/(convection_coefficient_/sqrt(y2_*x2_)))
							);
					*tINF_mno_M1=-1.0*(
									(y2_*z2_)/
									((z2_/(2*conduct_center_down_)) + 1/(convection_coefficient_/sqrt(y2_*x2_)))
									);
					*tINF_mno+=*tINF_mno_M1;				
				}
				else{
					*t_mno+=tINF_mno_M1_eff;
					*tINF_mno_M1=tINF_mno_M1_eff;
					*tINF_mno+=tINF_mno_M1_eff;					
				}
				
			}
			
			//************** Create T_m,n,o+1 **************
			if(EQ(z3_,0.0)){
//				*t_mno+=0;
//				*t_mno_P1=0;
			}
			else if (!EQ(z3_,-1.0))	   //if T_m,n,o+1 isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_up+conduct_convection_up+conduct_radiation_up))
							);	
					*t_mno_P1=-1.0*(
								 ((conduct_conduction_up+conduct_convection_up+conduct_radiation_up))
								 );	
				}
				else{
					*t_mno+=t_mno_P1_eff;
					*t_mno_P1=t_mno_P1_eff;
				}
			}
			else				//if T_m,n,o+1 is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(x2_*y2_)/
							((z2_/(2*conduct_center_up_)) + 1/(convection_coefficient_/sqrt(x2_*y2_)))
							);
					*tINF_mno_P1=-1.0*(
									(y2_*z2_)/
									((z2_/(2*conduct_center_up_)) + 1/(convection_coefficient_/sqrt(x2_*y2_)))
									);
					*tINF_mno+=*tINF_mno_P1;
				}
				else{
					*t_mno+=tINF_mno_P1_eff;
					*tINF_mno_P1=tINF_mno_P1_eff;
					*tINF_mno+=tINF_mno_P1_eff;	
				}
			}	
			
			
			//************** Create T_m,n-1,o **************
			if(EQ(y1_,0.0)){
//				*t_mno+=0;
//				*t_mn_M1_o=0;
			}
			else if (!EQ(y1_,-1.0))	   //if T_m,n-1,o isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_bottom+conduct_convection_bottom+conduct_radiation_bottom))
							);	
					*t_mn_M1_o=-1.0*(
								  ((conduct_conduction_bottom+conduct_convection_bottom+conduct_radiation_bottom))
								  );	
				}
				else{
					*t_mno+=t_mn_M1_o_eff;
					*t_mn_M1_o=t_mn_M1_o_eff;					
				}
			}
			else				//if T_m,n-1,o is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(x2_*z2_)/
							((y2_/(2*conduct_center_bottom_)) + 1/(convection_coefficient_/sqrt(x2_*z2_)))
							);
					*tINF_mn_M1_o=-1.0*(
									 (x2_*z2_)/
									 ((y2_/(2*conduct_center_bottom_)) + 1/(convection_coefficient_/sqrt(x2_*z2_)))
									 );
					*tINF_mno+=*tINF_mn_M1_o;
				}
				else{
					*t_mno+=tINF_mn_M1_o_eff;
					*tINF_mn_M1_o=tINF_mn_M1_o_eff;
					*tINF_mno+=tINF_mn_M1_o_eff;
				}
			}
			
			
			//************** Create T_m,n+1,o **************
			if(EQ(y3_,0.0)){
//				*t_mno+=0;
//				*t_mn_P1_o=0;
			}		
			else if (!EQ(y3_,-1.0))	   //if T_m,n+1,o isn't ambient air
			{
				if(!use_rc){
					*t_mno+=(
							((conduct_conduction_top+conduct_convection_top+conduct_radiation_top))
							);	
					*t_mn_P1_o=-1.0*(
								  ((conduct_conduction_top+conduct_convection_top+conduct_radiation_top))
								  );
				}
				else{
					*t_mno+=t_mn_P1_o_eff;
					*t_mn_P1_o=t_mn_P1_o_eff;
				}
					
			}
			else				//if T_m,n+1,o is ambient air
			{
				if(!use_rc){
					*t_mno+=(
							(x2_*z2_)/
							((y2_/(2*conduct_center_top_)) + 1/(convection_coefficient_/sqrt(x2_*z2_)))
							);
					*tINF_mn_P1_o=-1.0*(
									 (x2_*z2_)/
									 ((y2_/(2*conduct_center_top_)) + 1/(convection_coefficient_/sqrt(x2_*z2_)))
									 );
					*tINF_mno+=*tINF_mn_P1_o;
				}
				else{
					*t_mno+=tINF_mn_P1_o_eff;
					*tINF_mn_P1_o=tINF_mn_P1_o_eff;
					*tINF_mno+=tINF_mn_P1_o_eff;
				}
			}		
		}
		//this means that we are in a layer that does not have internal power generation (layers 2,3 or 4)
		if (source_chip_flp_==NULL) {
			*get_temperature()=*get_previous_temperature()*gamma_ + 0.0;
			
			
		} else {
			if(EQ(power_per_unit_area_,-1.0)){
				*get_temperature()=*get_previous_temperature()*gamma_ + source_chip_flp_->power_per_unit_area_*x2_*y2_*z2_;
			}
			else{
				*get_temperature()=*get_previous_temperature()*gamma_ + power_per_unit_area_*x2_*y2_*z2_;            
			}
		}
		
		
	}	
	else {
        cerr << "Model-unit error: Invalid governing equation: " << governing_equation_ << endl;
        exit(1);
    }
	/*
	//microcooler cold side
	else if (governing_equation_ == UCOOL_COLD_EQUATION) {
		    calc_ucool_resistance_th();
        if (!calc_solution_only) {
            //calculate Tm,n,o
			*t_mno = 1
			+((gamma_*conduct_left_*conduct_center_*z2_*y2_)/
			  ((x1_/2)*conduct_center_ + (x2_/2)*conduct_left_ + resist_left_*conduct_left_*conduct_center_))
			+((gamma_*conduct_right_*conduct_center_*z2_*y2_)/
			  ((x3_/2)*conduct_center_ + (x2_/2)*conduct_right_ + resist_right_*conduct_right_*conduct_center_))
			+((gamma_/ucool_rth_))
			+((gamma_*conduct_top_*conduct_center_*z2_*x2_)/
			  ((y3_/2)*conduct_center_ + (y2_/2)*conduct_top_ + resist_top_*conduct_top_*conduct_center_))
			+((gamma_*conduct_down_*conduct_center_*x2_*y2_)/
			  ((z1_/2)*conduct_center_ + (z2_/2)*conduct_down_ + resist_down_*conduct_down_*conduct_center_))
			+((gamma_*conduct_up_*conduct_center_*x2_*y2_)/
			  ((z3_/2)*conduct_center_ + (z2_/2)*conduct_up_ + resist_up_*conduct_up_*conduct_center_))
			+ (datalibrary_->config_data_->ucool_seebeck_*2*datalibrary_->config_data_->ucool_coupled_devices_
			   *datalibrary_->config_data_->ucool_current_*gamma_*x2_*y2_*z2_)/
			(source_ucool_flp_->width_*source_ucool_flp_->height_*y2_);
			
            //calculate  Tm-1,n,o
			*t_m_M1_no=-1*((gamma_*conduct_left_*conduct_center_*z2_*y2_)/
						  ((x1_/2)*conduct_center_ + (x2_/2)*conduct_left_ + resist_left_*conduct_left_*conduct_center_));
			
            //calculate Tm+1,n,o
            *t_m_P1_no=-1*(
						  (gamma_*conduct_right_*conduct_center_*z2_*y2_)/
						  ((x3_/2)*conduct_center_ + (x2_/2)*conduct_right_ + resist_right_*conduct_right_*conduct_center_)
						  );
			
            //calculate Tm,n-1,o
            *t_mn_M1_o=0;
			
            //calculate Tm,n+1,o
            *t_mn_P1_o=-1*(
						  (gamma_*conduct_top_*conduct_center_*z2_*x2_)/
						  ((y3_/2)*conduct_center_ + (y2_/2)*conduct_top_ + resist_top_*conduct_top_*conduct_center_)
						  );
			
            //calculate Tm,n,o-1
            *t_mno_M1=-1*(
						 (gamma_*conduct_down_*conduct_center_*x2_*y2_)/
						 ((z1_/2)*conduct_center_ + (z2_/2)*conduct_down_ + resist_down_*conduct_down_*conduct_center_)
						 );
			
            //calculate Tm,n,o+1
            *t_mno_P1=-1*(
						 (gamma_*conduct_up_*conduct_center_*x2_*y2_)/
						 ((z3_/2)*conduct_center_ + (z2_/2)*conduct_up_ + resist_up_*conduct_up_*conduct_center_)
						 );
			
			
            //calculate T_inf m,n,o
            *tINF_mno=0;
			
            //calculate Tvil,m,o
            *tVIL_mo=-1*(
						(gamma_/ucool_rth_)
						);
        }
		if(power_per_unit_area_==-1){
			*get_temperature()=*get_previous_temperature() + source_chip_flp_->power_per_unit_area_*gamma_*x2_*y2_*z2_;
		}
		else{
			*get_temperature()=*get_previous_temperature() + power_per_unit_area_*gamma_*x2_*y2_*z2_;            
		}
				
	} //microcooler inner layer
	
	else if (governing_equation_ == UCOOL_INNER_LAYER_EQUATION) {	
		    calc_ucool_resistance_th();
        if (!calc_solution_only) {
            // calculate Tm,n,o
            *t_mno=0;
			
            //calculate  Tm-1,n,o
            *t_m_M1_no=0;
			
            //calculate Tm+1,n,o
            *t_m_P1_no=0;
			
            //calculate Tm,n-1,o
            *t_mn_M1_o=-1*(
						  (gamma_/ucool_rth_)
						  );
			
            //calculate Tm,n+1,o
            *t_mn_P1_o=-1*(
						  (gamma_/ucool_rth_)
						  );
			
            //calculate Tm,n,o-1
            *t_mno_M1=0;
			
            //calculate Tm,n,o+1
            *t_mno_P1=0;
			
			//calculate T_inf m,n,o
            *tINF_mno=0;
			
            //calculate Tvil,m,o
            *tVIL_mo=1 + (
						 (gamma_/ucool_rth_)
						 ) +
				(
				 (gamma_/ucool_rth_)
				 );
        }
        //calculate solution data
        //note: in this case, previous solution data is TVILmo (not Tmno) (but it doesn't matter as 1.17 doesn't use Tmno)
        *get_temperature()=*get_previous_temperature()+
		(
		 (2*datalibrary_->config_data_->ucool_coupled_devices_
		  *datalibrary_->config_data_->ucool_current_*datalibrary_->config_data_->ucool_current_
		  *datalibrary_->config_data_->ucool_resistivity_
		  *gamma_
		  *x2_*z2_)/
		 (datalibrary_->config_data_->ucool_g_*source_ucool_flp_->width_*source_ucool_flp_->height_)
		 );
    } 	
	else if (governing_equation_ == UCOOL_HOT_EQUATION) {
		    calc_ucool_resistance_th();
        if (!calc_solution_only) {
            // calculate Tm,n,o
            *t_mno=   1+(
						(gamma_*conduct_left_*conduct_center_*z2_*y2_)/
						((x1_/2)*conduct_center_ + (x2_/2)*conduct_left_ + resist_left_*conduct_left_*conduct_center_)
						)
			+(
			  (gamma_*conduct_right_*conduct_center_*z2_*y2_)/
			  ((x3_/2)*conduct_center_ + (x2_/2)*conduct_right_ + resist_right_*conduct_right_*conduct_center_)
			  )
			+(
			  (gamma_*conduct_bottom_*conduct_center_*z2_*x2_)/
			  ((y1_/2)*conduct_center_ + (y2_/2)*conduct_bottom_ + resist_bottom_*conduct_bottom_*conduct_center_)
			  )
			+(
			  (gamma_/ucool_rth_)
			  )
			+(
			  (gamma_*conduct_down_*conduct_center_*x2_*y2_)/
			  ((z1_/2)*conduct_center_ + (z2_/2)*conduct_down_ + resist_down_*conduct_down_*conduct_center_)
			  )
			+(
			  (gamma_*conduct_up_*conduct_center_*x2_*y2_)/
			  ((z3_/2)*conduct_center_ + (z2_/2)*conduct_up_ + resist_up_*conduct_up_*conduct_center_)
			  )
			+ (-1*datalibrary_->config_data_->ucool_seebeck_*2*datalibrary_->config_data_->ucool_coupled_devices_
			   *datalibrary_->config_data_->ucool_current_*gamma_*x2_*y2_*z2_)/
			(source_ucool_flp_->width_*source_ucool_flp_->height_*y2_);
			
            //calculate  Tm-1,n,o
            *t_m_M1_no=-1*(
						  (gamma_*conduct_left_*conduct_center_*z2_*y2_)/
						  ((x1_/2)*conduct_center_ + (x2_/2)*conduct_left_ + resist_left_*conduct_left_*conduct_center_)
						  );
			
            //calculate Tm+1,n,o
            *t_m_P1_no=-1*(
						  (gamma_*conduct_right_*conduct_center_*z2_*y2_)/
						  ((x3_/2)*conduct_center_ + (x2_/2)*conduct_right_ + resist_right_*conduct_right_*conduct_center_)
						  );
			
            //calculate Tm,n-1,o
            *t_mn_M1_o=-1*(
						  (gamma_*conduct_bottom_*conduct_center_*z2_*x2_)/
						  ((y1_/2)*conduct_center_ + (y2_/2)*conduct_bottom_ + resist_bottom_*conduct_bottom_*conduct_center_)
						  );
			
            //calculate Tm,n+1,o
            *t_mn_P1_o=0;
			
            //calculate Tm,n,o-1
            *t_mno_M1=-1*(
						 (gamma_*conduct_down_*conduct_center_*x2_*y2_)/
						 ((z1_/2)*conduct_center_ + (z2_/2)*conduct_down_ + resist_down_*conduct_down_*conduct_center_)
						 );
			
            //calculate Tm,n,o+1
            *t_mno_P1=-1*(
						 (gamma_*conduct_up_*conduct_center_*x2_*y2_)/
						 ((z3_/2)*conduct_center_ + (z2_/2)*conduct_up_ + resist_up_*conduct_up_*conduct_center_)
						 );
			
			
            //calculate T_inf m,n,o
            *tINF_mno=0;
			
            //calculate Tvil,m,o
            *tVIL_mo= -1*(
						 (gamma_/ucool_rth_)
						 );
        }
		
        //calculate solution data
        *get_temperature()=*get_previous_temperature();
    }
	 */

	
	
}

/*
void model_unit::calc_hot_spot_count(){
	//if the model unit's temperature is less than 80% of the hotspot temperature
	//then record it
	if(get_temperature() < .80*datalibrary_->config_data_->hot_spot_temp_){
		not_hot_spot_=true;
	}	
	//otherwise if the unit just became a hotspot record that
	else if(get_temperature() > datalibrary_->config_data_->hot_spot_temp_ && not_hot_spot_){
		not_hot_spot_=false;
		hot_spot_count_++;	//increment the hot spot count for this unit
		if(source_chip_flp_!=NULL)
			source_chip_flp_->hot_spot_count_++;
		else if(source_ucool_flp_!=NULL)
			source_ucool_flp_->hot_spot_count_++;
	}
}

void model_unit::calc_hot_spot_duration(){
	if(get_temperature() > datalibrary_->config_data_->hot_spot_temp_)
		hot_spot_duration_+=datalibrary_->config_data_->default_time_increment_;
	if(source_chip_flp_!=NULL)
		source_chip_flp_->hot_spot_duration_+=datalibrary_->config_data_->default_time_increment_;
	else if(source_ucool_flp_!=NULL)
		source_ucool_flp_->hot_spot_duration_+=datalibrary_->config_data_->default_time_increment_;		
}
*/
void model_unit::calc_gamma(double time_increment, bool use_rc){
	if(!use_rc)
		gamma_=((row_*x2_*y2_*z2_*specific_heat_*6)/time_increment);
	else
		gamma_=((row_*x2_*y2_*z2_*specific_heat_eff_*9)/time_increment);
}

//return the maximum value of alpha (thermal diffusivity m^2/s)
//this will be used to determine the maximum/recommended timestep
double model_unit::calc_alpha_max(bool use_rc){
	double max_conductivity=-1.0;
	max_conductivity=MAX(max_conductivity, conduct_center_left_);
	max_conductivity=MAX(max_conductivity, conduct_center_right_);
	max_conductivity=MAX(max_conductivity, conduct_center_down_);
	max_conductivity=MAX(max_conductivity, conduct_center_up_);
	max_conductivity=MAX(max_conductivity, conduct_center_bottom_);	
	max_conductivity=MAX(max_conductivity, conduct_center_top_);	
	if(!use_rc)
		return(max_conductivity/(row_*specific_heat_));
	else
		return(max_conductivity/(row_*specific_heat_eff_));
	
}

double model_unit::calc_view_factor(double width, double height, double distance){
	double x_bar=width/distance;	//x_bar=x/l
	double y_bar=height/distance;	//y_bar=y/l
	return ( (2/ (M_PI*x_bar*y_bar))*
			 (
			  logf( sqrtf( (1+powf(x_bar,2.0))*(1+powf(y_bar,2.0)) ) )	//note: log=natural log
			  + ( x_bar*sqrt(1+powf(y_bar,2.0))*atan(x_bar/sqrt(1+powf(y_bar,2.0))) )
			  + ( y_bar*sqrt(1+powf(x_bar,2.0))*atan(y_bar/sqrt(1+powf(x_bar,2.0))) )
			  - ( x_bar*atan(x_bar) )
			  - ( y_bar*atan(y_bar) )
			  )
			 ); //compute view factor of aligned parallel rectangles (see Incropera/Dewitt pg 723
	
}

//calculate R_th/2
void model_unit::calc_ucool_resistance_th(){
    if (source_ucool_flp_==NULL) {
        if (governing_equation_!=2 && governing_equation_!=3 && governing_equation_!=4) {
            ucool_rth_=-1.0; //ucool_rth_ unused
            return;
        } else {
            cerr << "Fatal: ucool Region does not have source_ucool_flp_ assigned!" << endl;
            exit(1);
        }
    }
	
    double temp=0;
    for (uint32_t i=0;i<source_ucool_flp_->model_unit_percentages_.size();i++) {
		
        temp+=1/(1/source_ucool_flp_->model_unit_percentages_[i]);
    }
	
    ucool_rth_=(1/(4*datalibrary_->config_data_->ucool_conductivity_*datalibrary_->config_data_->ucool_coupled_devices_*
                   datalibrary_->config_data_->ucool_g_))*temp*(1/ucool_percentage_);
}



void model_unit::print_dyn_layer(int layer, sesctherm3Ddatalibrary* datalibrary_){
	
  dynamic_array<model_unit>* dyn_layer = datalibrary_->all_layers_info_[layer]->floorplan_layer_dyn_;
  
  cerr << " ***** PRINTING Tm-1,n,o for layer [" << layer << "*****" << endl;
  
  cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
  cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
  
  for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
    for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
      if((*dyn_layer)[x_itor][y_itor].t_m_M1_no!=NULL)
	cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_m_M1_no << "]   ";
      else
	cerr << "[" << setw(6) << 0 << "]   ";
    }
    cerr << endl;
  }
  cerr << endl << endl;
  
  cerr << " ***** PRINTING Tm,n,o for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_mno!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_mno << "]   ";
				else
					cerr << "[" << setw(6) << 0 << "]   ";
            }
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING Tm+1,n,o for layer [" << layer << "*****" << endl;

        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_m_P1_no!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_m_P1_no << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
    cerr << " ***** PRINTING Tm,n-1,o for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_mn_M1_o!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_mn_M1_o << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING Tm,n+1,o for layer [" << layer << "*****" << endl;

        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_mn_P1_o!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_mn_P1_o << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING Tm,n,o-1 for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_mno_M1!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_mno_M1 << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING Tm,n,o+1 for layer [" << layer << "*****" << endl;

        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].t_mno_P1!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].t_mno_P1 << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
		
	/*	
		//PRINT specific_heat_
		cerr << " ***** PRINTING specific_heat_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].specific_heat_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		//PRINT row_
		cerr << " ***** PRINTING row_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].row_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		
		//Printing Transfer Methods
		cerr << " ***** PRINTING transfer_methods_center_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (int)(*dyn_layer)[x_itor][y_itor].heat_transfer_methods_center_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;		
		
		//printing heat capacitance
		cerr << " ***** PRINTING lumped thermal capacitance for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].row_*
				(*dyn_layer)[x_itor][y_itor].x2_*
				(*dyn_layer)[x_itor][y_itor].y2_*
				(*dyn_layer)[x_itor][y_itor].z2_*
				(*dyn_layer)[x_itor][y_itor].specific_heat_
											<< "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		
		//PRINT CONDUCT CENTER (left, right, down, up, bottom, top)	
		cerr << " ***** PRINTING conduct_center_left_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_left_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		cerr << " ***** PRINTING conduct_center_right_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_right_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		cerr << " ***** PRINTING conduct_center_down_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_down_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		cerr << " ***** PRINTING conduct_center_up_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_up_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		cerr << " ***** PRINTING conduct_center_bottom_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_bottom_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		cerr << " ***** PRINTING conduct_center_top_ for layer [" << layer << "]*****" << endl;
		
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_top_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		
		return;

    cerr << " ***** PRINTING TVILm,o for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
                cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].tVIL_mo << "]   ";
            }
            cerr << endl;
        }
        cerr << endl << endl;
    
*/	
    cerr << " ***** PRINTING TINFm-1,n,o for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].tINF_m_M1_no!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_m_M1_no << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING TINFm,n,o for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				if((*dyn_layer)[x_itor][y_itor].tINF_mno!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_mno << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING TINFm+1,n,o for layer [" << layer << "*****" << endl;

        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].tINF_m_P1_no!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_m_P1_no << "]   ";
				else
					cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
    cerr << " ***** PRINTING TINFm,n-1,o for layer [" << layer << "*****" << endl;



        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].tINF_mn_M1_o!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_mn_M1_o << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING TINFm,n+1,o for layer [" << layer << "*****" << endl;



        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].tINF_mn_P1_o!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_mn_P1_o << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING TINFm,n,o-1 for layer [" << layer << "*****" << endl;



        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				if((*dyn_layer)[x_itor][y_itor].tINF_mno_M1!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_mno_M1 << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    
	
    cerr << " ***** PRINTING TINFm,n,o+1 for layer [" << layer << "*****" << endl;


        cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
        cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
        for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
            for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if((*dyn_layer)[x_itor][y_itor].tINF_mno_P1!=NULL)
                cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].tINF_mno_P1 << "]   ";
			else
				cerr << "[" << setw(6) << 0 << "]   ";
			}
            cerr << endl;
        }
        cerr << endl << endl;
    

		
		//PRINT x1_
		cerr << " ***** PRINTING x1_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].x1_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT x2_
		cerr << " ***** PRINTING x2_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].x2_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT x3_
		cerr << " ***** PRINTING x3_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].x3_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT y1_
		cerr << " ***** PRINTING y1_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].y1_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT y2_
		cerr << " ***** PRINTING y2_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].y2_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT y3_
		cerr << " ***** PRINTING y3_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].y3_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT z1_
		cerr << " ***** PRINTING z1_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].z1_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT z2_
		cerr << " ***** PRINTING z2_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].z2_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT z3_
		cerr << " ***** PRINTING z3_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].z3_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;				

		//PRINT WIDTH_
		cerr << " ***** PRINTING width_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].width_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		//PRINT HEIGHT_
		cerr << " ***** PRINTING height_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].height_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;	
		
		return;
	
	//PRINTING LEFTX (left, right, down, up, bottom, top)	
	cerr << " ***** PRINTING leftx_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].leftx_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;	
	
	
	//PRINTING BOTTOMY (left, right, down, up, bottom, top)	
	cerr << " ***** PRINTING bottomy_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].bottomy_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
			
	
	//PRINT CONDUCT CENTER (left, right, down, up, bottom, top)	
	cerr << " ***** PRINTING conduct_center_left_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_left_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	cerr << " ***** PRINTING conduct_center_right_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_right_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	cerr << " ***** PRINTING conduct_center_down_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_down_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	cerr << " ***** PRINTING conduct_center_up_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_up_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	cerr << " ***** PRINTING conduct_center_bottom_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_bottom_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	cerr << " ***** PRINTING conduct_center_top_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_center_top_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	
	//PRINT CONDUCT LEFT
	cerr << " ***** PRINTING conduct_left_ for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_left_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
	
	
	//PRINT CONDUCT RIGHT
	cerr << " ***** PRINTING conduct_right_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_right_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
  	//PRINT CONDUCT DOWN
	cerr << " ***** PRINTING conduct_down_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_down_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT CONDUCT UP
	cerr << " ***** PRINTING conduct_up_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_up_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT CONDUCT BOTTOM
	cerr << " ***** PRINTING conduct_bottom_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_bottom_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT CONDUCT TOP
	cerr << " ***** PRINTING conduct_top_ for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].conduct_top_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
 	
   	//PRINT defined_
    cerr << " ***** PRINTING defined_  for layer [" << layer << "]*****" << endl;
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].defined_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT flp_percentage
    cerr << " ***** PRINTING flp_percentage_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].flp_percentage_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	
	//PRINT resist_bottom
    cerr << " ***** PRINTING resist_bottom_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_bottom_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT resist_down
	
    cerr << " ***** PRINTING resist_down_  for layer [" << layer << "]*****" << endl;
	
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_down_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT resist_left
    cerr << " ***** PRINTING resist_left_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_left_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	//PRINT resist_right
    cerr << " ***** PRINTING resist_right_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_right_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	//PRINT resist_top
    cerr << " ***** PRINTING resist_top_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_top_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	//PRINT resist_up
    cerr << " ***** PRINTING resist_up_  for layer [" << layer << "]*****" << endl;
	
	
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].resist_up_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	
	//PRINT name
	cerr << " ***** PRINTING name_ for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
		//	cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].name_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	//PRINT source chip flp
if (datalibrary_->all_layers_info_[layer]->chip_floorplan_!=NULL){
	cerr << " ***** PRINTING source_chip_flp_->name_ for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			if ((*dyn_layer)[x_itor][y_itor].source_chip_flp_!=NULL)
				cerr << "[" << setw(6) << ((*dyn_layer)[x_itor][y_itor].source_chip_flp_)->name_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    }
	
	
	//PRINT solution data
    cerr << " ***** PRINTING get_temperature()  for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << *(*dyn_layer)[x_itor][y_itor].get_temperature() << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	//PRINT energy dat
if (datalibrary_->all_layers_info_[layer]->chip_floorplan_!=NULL){
    cerr << " ***** PRINTING energy_data_ for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;		
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			
			if ((*dyn_layer)[x_itor][y_itor].source_chip_flp_!=NULL){
				if((*dyn_layer)[x_itor][y_itor].power_per_unit_area_==-1)
					cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].source_chip_flp_->power_per_unit_area_*
						(*dyn_layer)[x_itor][y_itor].x2_*
	      				(*dyn_layer)[x_itor][y_itor].y2_*
	      				(*dyn_layer)[x_itor][y_itor].z2_ << "]   ";
				else
					cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].power_per_unit_area_*
	      				(*dyn_layer)[x_itor][y_itor].x2_*
	      				(*dyn_layer)[x_itor][y_itor].y2_*
	      				(*dyn_layer)[x_itor][y_itor].z2_ << "]   ";        
			}
		}
		cerr << endl;
	}
	
	
	cerr << endl << endl;
    }
	
	//PRINT gamma_
    cerr << " ***** PRINTING gamma_ for layer [" << layer << "]*****" << endl;
	cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
	cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
	
	for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
		for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
			cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].gamma_ << "]   ";
		}
		cerr << endl;
	}
	cerr << endl << endl;
    
	

    
		
	if(datalibrary_->all_layers_info_[layer]->ucool_floorplan_ != NULL){
		//PRINT source_ucool_flp_->name_
		cerr << " ***** PRINTING source_ucool_flp_->name_ for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				
				if ((*dyn_layer)[x_itor][y_itor].source_ucool_flp_!=NULL)
					cerr << "[" << setw(6) << ((*dyn_layer)[x_itor][y_itor].source_ucool_flp_)->name_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		
		//PRINT ucool percentage	
		cerr << " ***** PRINTING ucool_percentage_  for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].ucool_percentage_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
		
		//PRINT ucool_rth_	
		cerr << " ***** PRINTING ucool_rth_  for layer [" << layer << "]*****" << endl;
		cerr << "(" << layer << ").ncols()=" << dyn_layer->ncols() << endl;
		cerr << "(" << layer << ").nrows()=" << dyn_layer->nrows() << endl << endl;
		
		for (uint32_t y_itor=0;y_itor<dyn_layer->nrows();y_itor++) {
			for (uint32_t x_itor=0;x_itor<dyn_layer->ncols();x_itor++) {
				cerr << "[" << setw(6) << (*dyn_layer)[x_itor][y_itor].ucool_rth_ << "]   ";
			}
			cerr << endl;
		}
		cerr << endl << endl;
		
	}			
}

// FIXME: pass vector<SUElement_t> as parameter, not as return value
vector<double> model_unit::compute_average_temps(int flp_layer, dynamic_array<model_unit>& dyn_array,int sample_type, sesctherm3Ddatalibrary* datalibrary) {
  vector<double> temperature_map;
  vector<model_unit*> located_units;
  double average_temp=0;
  for(int i=0;i<(int)datalibrary->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_.size();i++){
    chip_flp_unit& flp_unit=datalibrary->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[i];
    model_unit::locate_model_units(dyn_array,
				   located_units,
				   flp_unit.leftx_,
				   flp_unit.bottomy_,
				   flp_unit.leftx_+flp_unit.width_,
				   flp_unit.bottomy_+flp_unit.height_,
				   datalibrary);
    
    double average_temp=0;
    //compute average temperature for the units
    for(int j=0;j<(int)located_units.size();j++){
      switch(sample_type){
      case GFX_CUR:
	if(located_units[j]->get_temperature()!=NULL){
	  average_temp+=*located_units[j]->get_temperature();
	}
	break;
     case  GFX_MIN:
	average_temp+=located_units[j]->min_temperature_;
	break;
      case GFX_MAX:
	average_temp+=located_units[j]->max_temperature_;
	break;
      case GFX_AVE:
	average_temp+=located_units[j]->ave_temperature_;
	break;
      default:
	break;
      }
    }									
    average_temp/=located_units.size();
    temperature_map.push_back(average_temp);	
  }

  return(temperature_map);
}

vector<double> model_unit::compute_average_powers(int flp_layer, dynamic_array<model_unit>& dyn_array, int32_t sample_type, sesctherm3Ddatalibrary* datalibrary) {

  vector<double> power_map;
  vector<model_unit*> located_units;
  double average_energy=0;

  for(int i=0;i<(int)datalibrary->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_.size();i++){
    chip_flp_unit& flp_unit=datalibrary->all_layers_info_[flp_layer]->chip_floorplan_->flp_units_[i];
    model_unit::locate_model_units(dyn_array,
				   located_units,
				   flp_unit.leftx_,
				   flp_unit.bottomy_,
				   flp_unit.leftx_+flp_unit.width_,
				   flp_unit.bottomy_+flp_unit.height_,
				   datalibrary);
    
    double average_energy=0;
    //compute average power for the units
    for(int j=0;j<(int)located_units.size();j++){
      switch(sample_type){
      case GFX_CUR:
	average_energy+=located_units[j]->energy_data_;
	break;
     case GFX_MIN:
	average_energy+=located_units[j]->min_energy_data_;
	break;
      case GFX_MAX:
	average_energy+=located_units[j]->max_energy_data_;
	break;
     case GFX_AVE:
	average_energy+=located_units[j]->ave_energy_data_;
	break;
      default:
	break;
      }
    }									
    average_energy/=located_units.size();
    power_map.push_back(average_energy);	
  }

  return(power_map);
}



//searches for a base unit within a given layer with leftx/bottomy coordinates specified
//returns NULL is no element was found

model_unit* model_unit::find_model_unit(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary) {

  std::map<double, int>::iterator itr;
  std::map<double, int>& coord_to_index_x = datalibrary->all_layers_info_[layer]->coord_to_index_x_;
  std::map<double, int>& coord_to_index_y = datalibrary->all_layers_info_[layer]->coord_to_index_y_;

  int32_t index_x=0;
  int32_t index_y=0;
	
  if(LT(leftx,datalibrary->all_layers_info_[layer]->leftx_) ||
     GT(leftx,datalibrary->all_layers_info_[layer]->leftx_+datalibrary->all_layers_info_[layer]->width_) ||
     LT(bottomy,datalibrary->all_layers_info_[layer]->bottomy_) ||
     GT(bottomy,datalibrary->all_layers_info_[layer]->bottomy_+datalibrary->all_layers_info_[layer]->height_)
     ) 
    return(NULL);
  
  if(!datalibrary->all_layers_info_[layer]->layer_used_)
    return(NULL);
  
  if ( (itr = find_if(coord_to_index_x.begin(), coord_to_index_x.end(), value_equals<double, int>(leftx))) == coord_to_index_x.end() ){
    //for(itr = coord_to_index_x.begin(); itr!=coord_to_index_x.end(); itr++)		
    //	cerr << "[" << itr->first << "][" << itr->second << "]" << endl;
    
    return(NULL);
  }else
    index_x=itr->second;
  
  if ( (itr = find_if(coord_to_index_y.begin(), coord_to_index_y.end(), value_equals<double, int>(bottomy))) == coord_to_index_y.end() ) 
    return(NULL);
  else
    index_y=itr->second;
  
  //	I(index_x <	(int)datalibrary->all_layers_info_[layer]->floorplan_layer_dyn_->nrows() && index_x>0 &&
  //		index_y < (int)datalibrary->all_layers_info_[layer]->floorplan_layer_dyn_->ncols() && index_y>0);
  
  return(&(*datalibrary->all_layers_info_[layer]->floorplan_layer_dyn_)[index_x][index_y]);
}



int model_unit::find_model_unit_xitor(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary) {
	
	std::map<double, int>::iterator itr;
	std::map<double, int>& coord_to_index_x = datalibrary->all_layers_info_[layer]->coord_to_index_x_;
	std::map<double, int>& coord_to_index_y = datalibrary->all_layers_info_[layer]->coord_to_index_y_;
	
	int index_x=0;
	int index_y=0;
	
	if ( (itr = find_if(coord_to_index_x.begin(), coord_to_index_x.end(), value_equals<double, int>(leftx))) == coord_to_index_x.end() )
		return(-1);
	
	index_x=itr->second;
	
	return index_x;
}


int model_unit::find_model_unit_yitor(double leftx, double bottomy, int32_t layer, sesctherm3Ddatalibrary* datalibrary){
	
	std::map<double, int>::iterator itr;
	std::map<double, int>& coord_to_index_x = datalibrary->all_layers_info_[layer]->coord_to_index_x_;
	std::map<double, int>& coord_to_index_y = datalibrary->all_layers_info_[layer]->coord_to_index_y_;
	
	int index_x=0;
	int index_y=0;
	
	if ( (itr = find_if(coord_to_index_y.begin(), coord_to_index_y.end(), value_equals<double, int>(bottomy))) == coord_to_index_y.end() ) 
		return(-1);
	
	index_y=itr->second;
	return(index_y);
}


void model_unit::locate_model_units(int layer,
				    vector<model_unit *>& located_units,
				    double leftx,
				    double bottomy,
				    double rightx,
				    double topy,
				    sesctherm3Ddatalibrary* datalibrary) {
	if(!located_units.empty()){
		located_units.clear();
	}
	dynamic_array<model_unit             >& layer_dyn=(*datalibrary->all_layers_info_[layer]->floorplan_layer_dyn_);
  for (uint32_t y_itor=0;y_itor<layer_dyn.nrows();y_itor++){
	  for (uint32_t x_itor=0;x_itor<layer_dyn.ncols();x_itor++){
		if(!layer_dyn[x_itor][y_itor].defined_)
			continue;
      if(GE(layer_dyn[x_itor][y_itor].leftx_  ,leftx) &&
	 LE(layer_dyn[x_itor][y_itor].rightx_ ,rightx) &&
	 GE(layer_dyn[x_itor][y_itor].bottomy_,bottomy) &&
	 LE(layer_dyn[x_itor][y_itor].topy_,topy))
	located_units.push_back(&(layer_dyn[x_itor][y_itor]));		
    }
  }
  
}


void model_unit::locate_model_units(dynamic_array<model_unit>& dyn_array,
									vector<model_unit *>& located_units,
									double leftx,
									double bottomy,
									double rightx,
									double topy,
									sesctherm3Ddatalibrary* datalibrary) {
	if(!located_units.empty()){
	located_units.clear();	//clear the vector
	}
	for (uint32_t y_itor=0;y_itor<dyn_array.nrows();y_itor++){
		for (uint32_t x_itor=0;x_itor<dyn_array.ncols();x_itor++){
			if(!dyn_array[x_itor][y_itor].defined_)
				continue;
			if(GE(dyn_array[x_itor][y_itor].leftx_  ,leftx) &&
			   LE(dyn_array[x_itor][y_itor].rightx_ ,rightx) &&
			   GE(dyn_array[x_itor][y_itor].bottomy_,bottomy) &&
			   LE(dyn_array[x_itor][y_itor].topy_   ,topy))
				located_units.push_back(&dyn_array[x_itor][y_itor]);		
		}
	}
	
}
