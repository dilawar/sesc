/*------------------------------------------------------------
 *                              CACTI 4.0
 *         Copyright 2005 Hewlett-Packard Development Corporation
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein, and
 * hereby grant back to Hewlett-Packard Company and its affiliated companies ("HP")
 * a non-exclusive, unrestricted, royalty-free right and license under any changes,
 * enhancements or extensions  made to the core functions of the software, including
 * but not limited to those affording compatibility with other hardware or software
 * environments, but excluding applications which incorporate this software.
 * Users further agree to use their best efforts to return to HP any such changes,
 * enhancements or extensions that they make and inform HP of noteworthy uses of
 * this software.  Correspondence should be provided to HP at:
 *
 *                       Director of Intellectual Property Licensing
 *                       Office of Strategy and Technology
 *                       Hewlett-Packard Company
 *                       1501 Page Mill Road
 *                       Palo Alto, California  94304
 *
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND HP DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL HP
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "cacti42_leakage.h"
#include "cacti42_def.h"
#include "cacti42_areadef.h"

/*dt: added for windows compatibility */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Box-Mueller Method */

double box_mueller(double std_var, double value)
{
	double temp;
	double temp1;
	double random;
	/* dt: drand48 not supported in windows
			random = drand48();
	*/
	random = rand();
	temp = sqrt((double)(-2.00 * (double)log(random)));
	random = rand();

	temp1 = cos(2.00 * M_PI * random);

	return(temp * temp1 * std_var * value);
}


/* ************************************************************************ */

/* Calculating the NMOS I Normalized Leakage From the BSIM Equation.*/
/* Also Using Box-Mueller to Find the Random Samples Due to Any Variation */
/* In any of the parameters like length, Vdd etc. */
/* ************************************************************************ */
double nmos_ileakage(double aspect_ratio, double Volt, double Vth0, double Tkelvin, double tox0)
{
	double Tox_Std_Array[No_of_Samples];
	double Vdd_Std_Array[No_of_Samples];
	double Tech_Std_Array[No_of_Samples];
	double Vthn_Std_Array[No_of_Samples];
	double Ileak_Std_Array[No_of_Samples];
	int i;
	double mean =0.0;

	if(Tox_Std || Tech_Std || Vdd_Std || Vthn_Std)
	{
		for(i =0; i<No_of_Samples;i++)
		{
		    Tox_Std_Array[i] = tox0;
		    Vdd_Std_Array[i] = Volt;
		    Tech_Std_Array[i] = tech_length0;
		    Vthn_Std_Array[i] = Vth0;
		}
	}

	if(Tox_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Tox_Std_Array[i] = tox0 + box_mueller(Tox_Std,tox0);
	  }
	if(Tech_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Tech_Std_Array[i] = tech_length0 + box_mueller(Tech_Std,tech_length0);
	  }
	if(Vdd_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Vdd_Std_Array[i] = Volt + box_mueller(Vdd_Std,Volt);
	  }
	if(Vthn_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Vthn_Std_Array[i] = Vth0 + box_mueller(Vthn_Std,Vth0);
	  }

	if(Tox_Std || Tech_Std || Vdd_Std || Vthn_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      {
	     Ileak_Std_Array[i] = nmos_ileakage_var(aspect_ratio, Vdd_Std_Array[i], Vthn_Std_Array[i],  Tkelvin,  Tox_Std_Array[i], Tech_Std_Array[i]);

	      }
	  }
	else
	  {
		return(nmos_ileakage_var(aspect_ratio,Volt,Vth0,Tkelvin, tox0, tech_length0));
	  }

	for(i =0; i<No_of_Samples;i++)
		mean += Ileak_Std_Array[i];
	mean = mean/(double)No_of_Samples;

	return mean;


}

double nmos_ileakage_var(double aspect_ratio, double Volt, double Vth0, double Tkelvin, double tox0, double tech_length)
{
	double Ileak;
	double Vthermal;
	double Vth,temp , Vnoff;
	double param1,param2,param3, param4,param5,param6;
	double temp1;
	param1 = (aspect_ratio * tech_length0 *M0n*Cox)/tech_length;
	/* Thermal Voltage */
	Vthermal =((Bk*Tkelvin)/Qparam);
	/* Vdd Fitting */
	temp =  Nb*(Volt- Volt0);
	param2 = exp(temp);
	param3 = 1-exp((-Volt/Vthermal));
	Vth =Vth0 + Vnthx * (Tkelvin-300);
	Vnoff = Vnoff0 + Nfix*(Vth0-Vthn);
	param4 = exp(((-fabs(Vth)-Vnoff)/(NEta*Vthermal)));
	temp = (tech_length0 - tech_length) * L_nmos_d ;
	param5 = exp(temp);
	temp1 = (tox0 - Tox) * Tox_nmos_e;
	param6 = exp(temp1);
	Ileak = param1*pow(Vthermal,2.0)*param2*param3*param4*param5*param6;

	return Ileak;
}

/* ************************************************************************ */

/* Calculating the PMOS I Normalized Leakage From the BSIM Equation.*/
/* Also Using Box-Mueller to Find the Random Samples Due to Any Variation */
/* In any of the parameters like length, Vdd etc. */
/* ************************************************************************ */
double pmos_ileakage(double aspect_ratio,double Volt, double Vth0,double Tkelvin,double tox0)
{

	double Tox_Std_Array[No_of_Samples];
	double Vdd_Std_Array[No_of_Samples];
	double Tech_Std_Array[No_of_Samples];
	double Vthp_Std_Array[No_of_Samples];
	double Ileak_Std_Array[No_of_Samples];
	int i;
	double mean =0.0;

	if(Tox_Std || Tech_Std || Vdd_Std || Vthp_Std) {
		for(i =0; i<No_of_Samples;i++)
	  	{
	  		Tox_Std_Array[i] = tox0;
	  		Vdd_Std_Array[i] = Volt;
	    	Tech_Std_Array[i] = tech_length0;
	    	Vthp_Std_Array[i] = Vth0;
	  	}
	}

	if(Tox_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Tox_Std_Array[i] = tox0 + box_mueller(Tox_Std,tox0);
	  }
	if(Tech_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Tech_Std_Array[i] = tech_length0 + box_mueller(Tech_Std,tech_length0);
	  }
	if(Vdd_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Vdd_Std_Array[i] = Volt + box_mueller(Vdd_Std,Volt);
	  }
	if(Vthp_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
	      	Vthp_Std_Array[i] = Vth0 + box_mueller(Vthp_Std,Vth0);
	  }

	if(Tox_Std || Tech_Std || Vdd_Std || Vthp_Std)
	  {
	    for(i =0; i<No_of_Samples;i++)
		    Ileak_Std_Array[i] =  pmos_ileakage_var(aspect_ratio, Vdd_Std_Array[i], Vthp_Std_Array[i],  Tkelvin,  Tox_Std_Array[i], Tech_Std_Array[i]);
	  }

	else
	  {
		return (pmos_ileakage_var(aspect_ratio,Volt,  Vth0, Tkelvin, tox0, tech_length0));
	  }

	for(i =0; i<No_of_Samples;i++)
		mean += Ileak_Std_Array[i];
	mean = mean/(double)No_of_Samples;

	return mean;

}

double pmos_ileakage_var(double aspect_ratio,double Volt, double Vth0,double Tkelvin,double tox0, double tech_length) {
	double Ileak;
	double Vthermal;
	double Vth, temp ,temp1,Vpoff;
	double param1,param2,param3,param4,param5,param6;
	param1 = (aspect_ratio * tech_length0 *M0p*Cox )/tech_length;
	/* Thermal Voltage */
	Vthermal =((Bk*Tkelvin)/Qparam);
	/* Vdd Fitting */
	temp =  Pb*(Volt- Volt0);
	param2 = exp(temp);
	param3 = 1-exp((-Volt/Vthermal));
	Vth =Vth0 + Vpthx * (Tkelvin-300);
	Vpoff = Vpoff0 + Pfix*(Vth0-Vthp);
	param4 = exp(((-fabs(Vth)-Vpoff)/(PEta*Vthermal)));
	temp = (tech_length0 - tech_length) * L_nmos_d ;
	param5 = exp(temp);
	temp1 = (tox0 - Tox) * Tox_nmos_e;
	param6 = exp(temp1);
	Ileak = param1*pow(Vthermal,2.0)*param2*param3*param4*param5*param6;

	return Ileak;
}

double simplified_cmos_leakage(double naspect_ratio,double paspect_ratio, double nVth0, double pVth0,
							   double *norm_nleak, double *norm_pleak)
{
	double sum;
	double pIleak,nIleak;
	double nVth,pVth,Vpoff,Vnoff;
	double nparam1,pparam1,nparam4,pparam4;

	nparam1 = naspect_ratio * precalc_nparamf;
	pparam1 = paspect_ratio * precalc_pparamf;

	nVth =nVth0 + precalc_Vnthx;
	pVth =pVth0 + precalc_Vpthx;
	Vnoff = Vnoff0 + Nfix*(nVth0-Vthn);
	Vpoff = Vpoff0 + Pfix*(pVth0-Vthp);
	nparam4 = exp(((-fabs(nVth)-Vnoff)*precalc_inv_nVthermal));
	pparam4 = exp(((-fabs(pVth)-Vpoff)*precalc_inv_pVthermal));

	*norm_nleak = precalc_nparamf*nparam4*precalc_nparaml;
	*norm_pleak = precalc_pparamf*pparam4*precalc_pparaml;

	nIleak = nparam1*nparam4*precalc_nparaml;
	pIleak = pparam1*pparam4*precalc_pparaml;

	sum = nIleak + pIleak;
	return sum;

}
double optimized_simplified_cmos_leakage(double naspect_ratio,double paspect_ratio, double nVth0, double pVth0,
							   double * nleak, double * pleak)
{
	double sum;
	double pIleak,nIleak;
	double nVth,pVth,Vpoff,Vnoff;
	double nparam1,pparam1,nparam4,pparam4;

	nparam1 = naspect_ratio * precalc_nparamf;
	pparam1 = paspect_ratio * precalc_pparamf;

	nVth =nVth0 + precalc_Vnthx;
	pVth =pVth0 + precalc_Vpthx;
	Vnoff = Vnoff0 + Nfix*(nVth0-Vthn);
	Vpoff = Vpoff0 + Pfix*(pVth0-Vthp);
	nparam4 = exp(((-fabs(nVth)-Vnoff)/(NEta*precalc_Vthermal)));
	pparam4 = exp(((-fabs(pVth)-Vpoff)/(PEta*precalc_Vthermal)));

	nIleak = nparam1*nparam4*precalc_nparaml;
	pIleak = pparam1*pparam4*precalc_pparaml;
	(*nleak) = precalc_nparamf*nparam4*precalc_nparaml;
	(*pleak) = precalc_pparamf*pparam4*precalc_pparaml;

	sum = nIleak + pIleak;
	return sum;

}
double simplified_nmos_leakage(double naspect_ratio, double nVth0)
{
	//double sum;
	double nIleak;
	double nVth,Vnoff;
	double nparam1,nparam4;

	if(have_leakage_params) {
		nparam1 = naspect_ratio * precalc_nparamf;
		nVth =nVth0 + precalc_Vnthx;
		Vnoff = Vnoff0 + Nfix*(nVth0-Vthn);
		nparam4 = exp(((-fabs(nVth)-Vnoff)/(NEta*precalc_Vthermal)));
		nIleak = nparam1*nparam4*precalc_nparaml;
	}
	else {
		nIleak = 0;
	}

	return nIleak;
}

double simplified_pmos_leakage(double paspect_ratio, double pVth0)
{
	//double sum;
	double pIleak;
	double pVth,Vpoff;
	double pparam1,pparam4;

	if(have_leakage_params) {
		pparam1 = paspect_ratio * precalc_pparamf;
		pVth =pVth0 + precalc_Vpthx;
		Vpoff = Vpoff0 + Pfix*(pVth0-Vthp);
		pparam4 = exp(((-fabs(pVth)-Vpoff)/(PEta*precalc_Vthermal)));
		pIleak = pparam1*pparam4*precalc_pparaml;
	}
	else {
		pIleak = 0;
	}

	return pIleak;
}

void precalc_leakage_params(double Volt,double Tkelvin,double tox0, double tech_length) {
	double temp1, temp2, temp3;
	double precalc_param3, precalc_param5, precalc_param6;

	precalc_Vnthx = Vnthx * (Tkelvin-300);
	precalc_Vpthx = Vpthx * (Tkelvin-300);
	/* Thermal Voltage */
	precalc_Vthermal =((Bk*Tkelvin)/Qparam);
	precalc_inv_nVthermal = 1.0/(NEta *precalc_Vthermal);
	precalc_inv_pVthermal = 1.0/(PEta *precalc_Vthermal);
	precalc_nparamf = tech_length0 *M0n*Cox /tech_length;
	precalc_pparamf = tech_length0 *M0p*Cox /tech_length;
	/* Vdd Fitting */
	temp1 =  Nb*(Volt- Volt0);
	precalc_nparam2 = exp(temp1);
	temp1 =  Pb*(Volt- Volt0);
	precalc_pparam2 = exp(temp1);
	precalc_param3 = 1-exp((-Volt/precalc_Vthermal));

	temp2 = (tech_length0 - tech_length) * L_nmos_d ;
	precalc_param5 = exp(temp2);
	temp3 = (tox0 - Tox) * Tox_nmos_e;
	precalc_param6 = exp(temp3);
	precalc_nparaml = pow(precalc_Vthermal,2.0)*precalc_nparam2*precalc_param3*precalc_param5*precalc_param6;
	precalc_pparaml = pow(precalc_Vthermal,2.0)*precalc_pparam2*precalc_param3*precalc_param5*precalc_param6;
}

void init_tech_params_default_process()
{
	Cndiffarea = 0.137e-15;
    Cpdiffarea = 0.343e-15;
    Cndiffside = 0.275e-15;
	Cpdiffside = 0.275e-15;
	Cndiffovlp = 0.138e-15;
	Cpdiffovlp = 0.138e-15;
	Cnoxideovlp = 0.263e-15;
    Cpoxideovlp = 0.338e-15;
    Leff = 0.8;
    inv_Leff = 1.25;
    Cgate = 1.95e-15;
	Cgatepass = 1.45e-15;
    Cpolywire = 0.25e-15;
    Rnchannelstatic = 25800;
    Rpchannelstatic = 61200;
    Rnchannelon = 8751;
    Rpchannelon = 20160;
    Wdecdrivep = 360;
	Wdecdriven = 240;
    Wworddrivemax = 100;
    Wmemcella = 0.9;
    Wmemcellpmos = 0.65;
    Wmemcellnmos = 2.0;
	Wmemcellbscale = 2;
	Wpchmax = 25.0;
    Wcompinvp1 = 10.0;
	Wcompinvn1 = 6.0;
	Wcompinvp2 = 20.0;
	Wcompinvn2 = 12.0;
	Wcompinvp3 = 40.0;
    Wcompinvn3 = 24.0;
    Wevalinvp = 80.0;
    Wevalinvn = 40.0;
    Wfadriven = 50.0;
    Wfadrivep = 100.0;
    Wfadrive2n = 200.0;
	Wfadrive2p = 400.0;
    Wfadecdrive1n = 5.0;
    Wfadecdrive1p = 10.0;
    Wfadecdrive2n = 20.0;
    Wfadecdrive2p = 40.0;
    Wfadecdriven = 50.0;
    Wfadecdrivep = 100.0;
    Wfaprechn = 6.0;
    Wfaprechp = 10.0;
    Wdummyn = 10.0;
    Wdummyinvn = 60.0;
    Wdummyinvp = 80.0;
    Wfainvn = 10.0;
	Wfainvp = 20.0;
	Waddrnandn = 50.0;
	Waddrnandp = 50.0;
	Wfanandn = 20.0;
    Wfanandp = 30.0;
    Wfanorn = 5.0;
	Wfanorp = 10.0;
    Wdecnandn = 10.0;
    Wdecnandp = 30.0;
    Wcompn = 10.0;
    Wcompp = 30.0;
    Wmuxdrv12n = 60.0;
    Wmuxdrv12p = 100.0;
    Wsenseextdrv1p = 80.0;
    Wsenseextdrv1n = 40.0;
    Wsenseextdrv2p = 240.0;
	Wsenseextdrv2n = 160.0;
    krise = 0.4e-9;
    tsensedata = 5.8e-10;
    psensedata = 0.02e-9;
    tsensescale = 0.02e-10;
    tsensetag = 2.6e-10;
    psensetag = 0.016e-9;
    tfalldata = 7e-10;
    tfalltag = 7e-10;
    BitWidth = 7.746*0.8;
    BitHeight = 2*7.746*0.8;
    Cout = 0.5e-12;
	Widthptondiff = 3.2;
    Widthtrack = 3.2*0.8;
    Widthcontact = 1.6;
    Wpoly = 0.8;
    ptocontact = 0.4;
    stitch_ramv = 6.0;
    BitHeight1x1 = 2*7.746*0.8;
    stitch_ramh = 12.0;
    BitWidth1x1 = 7.746*0.8;
    WidthNOR1 = 11.6;
    WidthNOR2 = 13.6;
    WidthNOR3 = 20.8;
    WidthNOR4 = 28.8;
    WidthNOR5 = 34.4;
    WidthNOR6 = 41.6;
    Predec_height1 = 140.8;
    Predec_width1 = 270.4;
    Predec_height2 = 140.8;
    Predec_width2 = 539.2;
    Predec_height3 = 281.6;
    Predec_width3 = 584.0;
    Predec_height4 = 281.6;
    Predec_width4 = 628.8;
    Predec_height5 = 422.4;
    Predec_width5 = 673.6;
    Predec_height6 = 422.4;
    Predec_width6 = 718.4;
    Wwrite = 1.2;
    SenseampHeight = 152.0;
    OutdriveHeight = 200.0;
    FAOutdriveHeight = 229.2;
    FArowWidth = 382.8;
    CAM2x2Height_1p = 48.8;
    CAM2x2Width_1p = 44.8;
    CAM2x2Height_2p = 80.8;
    CAM2x2Width_2p = 76.8;
    DatainvHeight = 25.6;
    Wbitdropv = 30.0;
    decNandWidth = 34.4;
    FArowNANDWidth = 71.2;
    FArowNOR_INVWidth = 28.0;

    FAHeightIncrPer_first_rw_or_w_port = 16.0;
    FAHeightIncrPer_later_rw_or_w_port = 16.0;
    FAHeightIncrPer_first_r_port = 12.0;
    FAHeightIncrPer_later_r_port = 12.0;
    FAWidthIncrPer_first_rw_or_w_port = 16.0;
    FAWidthIncrPer_later_rw_or_w_port = 9.6;
    FAWidthIncrPer_first_r_port = 12.0;
    FAWidthIncrPer_later_r_port = 9.6;
}

void init_tech_params(double technology) {

	double tech = technology * 1000.0;
	Tkelvin = 383.15;

	have_leakage_params = 1;

	//tech = (int) ceil(technology * 1000.0);

	if (tech < 181 && tech > 179) {
		process_tech = 0.18;
		tech_length0 = 180E-9;
		M0n =   3.5E-2;      /* Zero Bias Mobility for N-Type */
		M0p =  8.0E-3 ;      /* Zero Bias Mobility for P-Type */
		Tox = 3.5E-9;

		Cox = (Eox/Tox);     /* Gate Oxide Capacitance per unit area */
		Vnoff0 = 7.5E-2 ;    /* Empirically Determined Model Parameter for N-Type */
	                         /* FIX ME */
		Vpoff0 = -2.8e-2 ;   /* Empirically Determined Model Parameter for P-Type */
  	  	Nfix =  0.22;        /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
  		Pfix =  0.17 ;       /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */

  		Vthn =  0.3979 ;     /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) (original) */
  		Vthp =  0.4659 ;     /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) (original) */

  		Vnthx = -1.0E-3 ;    /* In the Equation Vth = Vth0 +Vnthx*(T-300) */
  		Vpthx =  -1.0E-3 ;   /* In the Equation Vth = Vth0 +Vpthx*(T-300) */
  		Vdd_init=  2.0 ;     /* Default Vdd. Can be Changed in leakage.c */
  		Volt0 =  1.7 ;

  		Na    =  -1.5 ;      /* Empirical param for the Vdd fit */
  		Nb    =   1.2 ;      /* Empirical param for the Vdd fit */
  		Pa    =   5.0 ;      /* Empirical param for the Vdd fit */
  		Pb    =   0.75 ;     /* Empirical param for the Vdd fit */
  		NEta =  1.5 ;        /* Sub-threshold Swing Co-efficient N-Type */
  		PEta =  1.6  ;       /* Sub-threshold Swing Co-efficient P-Type */

		/* gate Vss */
  		Vth0_gate_vss= 0.65;
  		aspect_gate_vss= 5;

		/*drowsy cache*/
  		Vdd_low= 0.6;

		/*RBB*/
  		k1_body_n= 0.5613;
  		k1_body_p= 0.5560;
  		vfi = 0.6;

  		VSB_NMOS= 0.5;
  		VSB_PMOS= 0.5;

		/* dual VT*/
  		Vt_cell_nmos_high= 0.45  ;
  		Vt_cell_pmos_high= 0.5;
  		Vt_bit_nmos_low = 0.35;
  		Vt_bit_pmos_low = 0.4;

  		L_nmos_d  = 0.1E+9;    /* Adjusting Factor for Length */
  		Tox_nmos_e  = 2.6E+9;  /* Adjusting Factor for Tox */
  		L_pmos_d  = 0.1E+9;    /* Adjusting Factor for Length */
  		Tox_pmos_e  = 2.6E+9;  /* Adjusting Factor for Tox */
	}

	/* TECH_POINT130nm */

	else if (tech < 131 && tech > 129)  {

		process_tech = 0.13;
  		tech_length0 = 130E-9;
  		M0n =  1.34E-2;      /* Zero Bias Mobility for N-Type */
  		M0p =  5.2E-3 ;      /* Zero Bias Mobility for P-Type */
  		//Tox =  3.3E-9;
  		Tox = 2.52731e-09;
  		Cox =  (Eox/Tox);    /* Gate Oxide Capacitance per unit area */
  		//Vnoff0 = -6.2E-2 ;   /* Empirically Determined Model Parameter for N-Type */
  		Vnoff0 = -1.68E-1 ;   /* Empirically Determined Model Parameter for N-Type */
  		//Vpoff0 = -0.1;       /* Empirically Determined Model Parameter for P-Type */
  		Vpoff0 = -0.28;       /* Empirically Determined Model Parameter for P-Type */
  		Nfix =   0.16 ;      /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
  		Pfix =  0.13 ;       /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
  		Vthn =  0.3353;      /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
  		Vthp =  0.3499 ;     /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
  		Vnthx = -0.85E-3;    /* In the Equation Vth = Vth0 +Vnthx*(T-300) */
  		Vpthx = -0.1E-3;     /* In the Equation Vth = Vth0 +Vpthx*(T-300) */
  		//Vdd_init =  1.5 ;    /* Default Vdd. */
  		//Vdd_init =  1.08 ;    /* Default Vdd. */
		Vdd_init = 1.3;
  		Volt0 =  1.2;

  		Na  =    3.275 ;     /* Empirical param for the Vdd fit */
  		Nb  =     1.1 ;      /* Empirical param for the Vdd fit */
  		Pa  =     4.65 ;     /* Empirical param for the Vdd fit */
  		Pb  =     2.2 ;      /* Empirical param for the Vdd fit */
  		NEta =  1.6 ;        /* Sub-threshold Swing Co-efficient N-Type */
  		PEta = 1.8   ;       /* Sub-threshold Swing Co-efficient P-Type */


		/* gate Vss */
  		Vth0_gate_vss = 0.55;
  		aspect_gate_vss = 5;

		/*drowsy cache*/
  		Vdd_low =  0.45;

		/*RBB*/
  		k1_body_n = 0.3662;
  		k1_body_p = 0.4087;
  		vfi =  0.55;
  		VSB_NMOS = 0.45;
  		VSB_PMOS = 0.45;

		/* dual VT*/
  		Vt_cell_nmos_high = 0.38  ;
  		Vt_cell_pmos_high=  0.4;
  		Vt_bit_nmos_low = 0.28;
  		Vt_bit_pmos_low = 0.29;


  		L_nmos_d  = 0.285E+9;  /* Adjusting Factor for Length */
  		Tox_nmos_e  = 4.3E+9;  /* Adjusting Factor for Tox */
  		L_pmos_d  = 0.44E+9;   /* Adjusting Factor for Length */
  		Tox_pmos_e  = 5.0E+9;  /* Adjusting Factor for Tox */
	}

	/* TECH_POINT100nm */

     else if (tech < 101 && tech > 99)  {

  		process_tech = 0.10;
  		tech_length0 = 100E-9;
  		M0n = 1.8E-2 ;      /* Zero Bias Mobility for N-Type */
  		M0p = 5.5E-3  ;     /* Zero Bias Mobility for P-Type */
  		Tox = 2.5E-9 ;
  		Cox = (Eox/Tox);    /* Gate Oxide Capacitance per unit area */
  		Vnoff0 = -2.7E-2;   /* Empirically Determined Model Parameter for N-Type */
  		                    /* FIX ME */
  		Vpoff0 = -1.04E-1;  /* Empirically Determined Model Parameter for P-Type */
  		Nfix  = 0.18  ;     /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
  		Pfix =  0.14   ;    /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
  		Vthn =  0.2607  ;   /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
  		Vthp =  0.3030 ;    /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
  		Vnthx = -0.77E-3 ;  /* In the Equation Vth = Vth0 +Vnthx*(T-300) */
  		Vpthx =  -0.72E-3;  /* In the Equation Vth = Vth0 +Vpthx*(T-300) */
  		Vdd_init = 1.2 ;    /* Default Vdd. Can be Changed for different parts in leakage.c */
  		Volt0 =  1.0;
  		Na   =   3.6 ;      /* Empirical param for the Vdd fit */
  		Nb   =   2.5  ;     /* Empirical param for the Vdd fit */
  		Pa   =    4.49 ;    /* Empirical param for the Vdd fit */
  		Pb   =    2.4 ;     /* Empirical param for the Vdd fit */

  		NEta =  1.7 ;       /* Sub-threshold Swing Co-efficient N-Type */
  		PEta =  1.7  ;      /* Sub-threshold Swing Co-efficient P-Type */


		/* gate Vss */
  		Vth0_gate_vss = 0.50;
  		aspect_gate_vss = 5;

		/*drowsy cache*/
  		Vdd_low =  0.36;

		/*RBB*/
  		k1_body_n =  0.395;
  		k1_body_p =  0.391;
  		vfi = 0.5;
  		VSB_NMOS =  0.4;
  		VSB_PMOS =  0.4 ;

		/* dual VT*/
  		Vt_cell_nmos_high =  0.31  ;
  		Vt_cell_pmos_high =  0.35;
  		Vt_bit_nmos_low  = 0.21;
  		Vt_bit_pmos_low = 0.25;


  		L_nmos_d  = 0.102E+9;   /* Adjusting Factor for Length */
  		Tox_nmos_e  = 2.5E+9;   /* Adjusting Factor for Tox */
  		L_pmos_d  = 0.21E+9;    /* Adjusting Factor for Length */
  		Tox_pmos_e  = 3.1E+9;   /* Adjusting Factor for Tox */

 	} else if (tech > 69 && tech < 71)  {

		process_tech = 0.07;
 		tech_length0 = 70E-9;
 		M0n =  3.5E-2 ;     /* Zero Bias Mobility for N-Type */
 		M0p = 8.0E-3 ;      /* Zero Bias Mobility for P-Type */
 		Tox = 1.0E-9;
 		Cox = (Eox/Tox) ;   /* Gate Oxide Capacitance per unit area */
 		Vnoff0 = -3.9E-2 ;  /* Empirically Determined Model Parameter for N-Type */
 		                    /* FIX ME */
 		Vpoff0 = -4.35E-2;  /* Empirically Determined Model Parameter for P-Type */
 		Nfix =  -0.05 ;     /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
 		Pfix =  0.07 ;      /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
 		Vthn =  0.1902  ;   /* In the equation Voff = Vnoff0 +Nfix*(Vth0-Vthn) */
 		Vthp =  0.2130 ;    /* In the equation Voff = Vpoff0 +Pfix*(Vth0-Vthp) */
 		Vnthx = -0.49E-3 ;  /* In the Equation Vth = Vth0 +Vnthx*(T-300) */
 		Vpthx =  -0.54E-3;  /* In the Equation Vth = Vth0 +Vpthx*(T-300) */
 		Vdd_init =  1.0;    /* Default Vdd. Can be Changed in leakage.c */
 		Volt0 =  0.8;
 		Na   =   -2.94 ;    /* Empirical param for the Vdd fit */
 		Nb   =    2.0  ;    /* Empirical param for the Vdd fit */
 		Pa   =    -2.245;   /* Empirical param for the Vdd fit */
 		Pb   =    2.89;     /* Empirical param for the Vdd fit */
 		NEta =  1.3;        /* Sub-threshold Swing Co-efficient N-Type */
 		PEta  = 1.4 ;       /* Sub-threshold Swing Co-efficient P-Type */

		/*gate leakage factor for 70nm*/
 		nmos_unit_leakage = 53e-12;
 		a_nmos_vdd = 7.75;
 		b_nmos_t =  0.15e-12;
 		c_nmos_tox =  11.75e9;

		pmos_unit_leakage = 22.9e-12;
 		a_pmos_vdd = 12;
 		b_pmos_t =  0.086e-12;
 		c_pmos_tox = 11.25e9;

		/* gate Vss */
 		Vth0_gate_vss =  0.45;
 		aspect_gate_vss =  5;

 	 	/*drowsy cache*/
 		Vdd_low =  0.3;

		/*RBB*/
 		k1_body_n = 0.37;
 		k1_body_p = 0.38;
 		vfi = 0.45;
 		VSB_NMOS = 0.35 ;
 		VSB_PMOS = 0.35;

		/* dual VT*/
 		Vt_cell_nmos_high =  0.29 ;
 		Vt_cell_pmos_high =  0.31;
 		Vt_bit_nmos_low = 0.19;
 		Vt_bit_pmos_low = 0.21;

 		L_nmos_d  = 0.1E+9;    /* Adjusting Factor for Length */
 		Tox_nmos_e  = 3.0E+9;  /* Adjusting Factor for Tox */
 		L_pmos_d  = 0.225E+9;  /* Adjusting Factor for Length */
 		Tox_pmos_e  = 4.0E+9;  /* Adjusting Factor for Tox */

	}

	else {
		have_leakage_params = 0;
		//printf("\n ERROR: Technology parameters not known for %3.4f feature size \n    (Parameters available for 0.18u, 0.13, 0.10, and 0.07u technologies)\nExiting...\n\n");
		//exit(0);
	}

	//v4.1: Specifying all resistances, capacitances, device/wire dimensions in terms of input tech node.
      //Presently derived from original 0.8 micron process

	FUDGEFACTOR = 0.8/technology;//Since 0.8 micron is the base process presently

	FEATURESIZE = technology;

	Cndiffarea = Cndiffarea * FUDGEFACTOR;//Since this is a F/micron2 capacitance, this will scale up by FUDGEFACTOR

	Cpdiffarea = Cpdiffarea * FUDGEFACTOR;//Since this is a F/micron2 capacitance, this will scale up by FUDGEFACTOR

      Cndiffside = Cndiffside;//Since this is a F/micron capacitance, this doesn't scale

	Cpdiffside = Cpdiffside;//Since this is a F/micron capacitance, this doesn't scale

	Cndiffovlp = Cndiffovlp;//Since this is a F/micron capacitance, this doesn't scale

	Cpdiffovlp = Cpdiffovlp;//Since this is a F/micron capacitance, this doesn't scale

      Cnoxideovlp = Cnoxideovlp;//Since this is a F/micron capacitance, this doesn't scale

	Cpoxideovlp = Cpoxideovlp;//Since this is a F/micron capacitance, this doesn't scale

      Leff = 0.8/FUDGEFACTOR;//Presently the tech parameters are based off a 0.8 micron process
	//If the technology node changes, then the 0.8 should be replaced appropriately.

      inv_Leff = inv_Leff * FUDGEFACTOR;//Since this is inverse of transistor length

	Cgate = Cgate * FUDGEFACTOR;//Since this is a F/micron2 capacitance, this will scale up by FUDGEFACTOR

	Cgatepass = Cgatepass * FUDGEFACTOR;//Since this is a F/micron2 capacitance, this will scale up by FUDGEFACTOR

      Cpolywire = Cpolywire * FUDGEFACTOR;//Since this is a F/micron2 capacitance, this will scale up by FUDGEFACTOR

      Rnchannelstatic	= Rnchannelstatic / FUDGEFACTOR;//Since this is an ohm-micron resistance, this will scale down by FUDGEFACTOR

      Rpchannelstatic	= Rpchannelstatic / FUDGEFACTOR;//Since this is an ohm-micron resistance, this will scale down by FUDGEFACTOR

      Rnchannelon	= Rnchannelon / FUDGEFACTOR;//Since this is an ohm-micron resistance, this will scale down by FUDGEFACTOR

      Rpchannelon	= Rpchannelon / FUDGEFACTOR;//Since this is an ohm-micron resistance, this will scale down by FUDGEFACTOR

	Wdecdrivep = 450 * FEATURESIZE;//this was 360 micron for the 0.8 micron process

      Wdecdriven = 300 * FEATURESIZE;//this was 240 micron for the 0.8 micron process

	  Wworddrivemax = 125 * FEATURESIZE; //this was 100 micron for the 0.8 micron process

	Wmemcella = 1.125 * FEATURESIZE;//this was 0.9 micron for the 0.8 micron process

      Wmemcellpmos = 0.8125 * FEATURESIZE;//this was 0.65 micron for the 0.8 micron process

	Wmemcellnmos = 2.5 * FEATURESIZE;//this was 2 micron for the 0.8 micron process
    //v4.2
	//Wpchmax = 31.25;//this was 25 micron for the 0.8 micron process
	Wpchmax = 31.25 * FEATURESIZE;//this was 25 micron for the 0.8 micron process

	Wcompinvp1 = 12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wcompinvn1 = 7.5 * FEATURESIZE;//this was 6 micron for the 0.8 micron process

	Wcompinvp2 = 25 * FEATURESIZE;//this was 20 micron for the 0.8 micron process

	Wcompinvn2 = 15 * FEATURESIZE;//this was 12 micron for the 0.8 micron process

	Wcompinvp3 = 50 * FEATURESIZE;//this was 40 micron for the 0.8 micron process

	Wcompinvn3 = 30 * FEATURESIZE;//this was 24 micron for the 0.8 micron process

	Wevalinvp =  100 * FEATURESIZE;//this was 80 micron for the 0.8 micron process

	Wevalinvn = 50 * FEATURESIZE;//this was 40 micron for the 0.8 micron process

	Wfadriven = 62.5 * FEATURESIZE;//this was 50 micron for the 0.8 micron process

	Wfadrivep = 125 * FEATURESIZE;//this was 100 micron for the 0.8 micron process

	Wfadrive2n = 250 * FEATURESIZE;//this was 200 micron for the 0.8 micron process

	Wfadrive2p = 500 * FEATURESIZE;//this was 400 micron for the 0.8 micron process

	Wfadecdrive1n = 6.25 * FEATURESIZE;//this was 5 micron for the 0.8 micron process

	Wfadecdrive1p = 12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wfadecdrive2n = 25 * FEATURESIZE;//this was 20 micron for the 0.8 micron process

	Wfadecdrive2p = 50 * FEATURESIZE;//this was 40 micron for the 0.8 micron process

	Wfadecdriven =  62.5 * FEATURESIZE;//this was 50 micron for the 0.8 micron process

	Wfadecdrivep =  125 * FEATURESIZE;//this was 100 micron for the 0.8 micron process

	Wfaprechn =  7.5 * FEATURESIZE;//this was 6 micron for the 0.8 micron process

	Wfaprechp = 12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wdummyn =  12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wdummyinvn = 75 * FEATURESIZE;//this was 60 micron for the 0.8 micron process

	Wdummyinvp = 100 * FEATURESIZE;//this was 80 micron for the 0.8 micron process

	Wfainvn = 12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wfainvp = 25 * FEATURESIZE;//this was 20 micron for the 0.8 micron process

	Waddrnandn = 62.5 * FEATURESIZE;//this was 50 micron for the 0.8 micron process

	Waddrnandp = 62.5 * FEATURESIZE;//this was 50 micron for the 0.8 micron process

	Wfanandn = 25 * FEATURESIZE;//this was 20 micron for the 0.8 micron process

	Wfanandp = 37.5 * FEATURESIZE;//this was 30 micron for the 0.8 micron process

	Wfanorn = 6.25 * FEATURESIZE;//this was 5 micron for the 0.8 micron process

	Wfanorp = 12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wdecnandn =  12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wdecnandp =  37.5 * FEATURESIZE;//this was 30 micron for the 0.8 micron process

	Wcompn =  12.5 * FEATURESIZE;//this was 10 micron for the 0.8 micron process

	Wcompp =  37.5 * FEATURESIZE;//this was 30 micron for the 0.8 micron process

	Wmuxdrv12n = 75 * FEATURESIZE;//this was 60 micron for the 0.8 micron process

	Wmuxdrv12p = 125 * FEATURESIZE;//this was 100 micron for the 0.8 micron process

	Wsenseextdrv1p = 100 * FEATURESIZE;//this was 80 micron for the 0.8 micron process

	Wsenseextdrv1n = 50 * FEATURESIZE;//this was 40 micron for the 0.8 micron process

	Wsenseextdrv2p =  300 * FEATURESIZE;//this was 240 micron for the 0.8 micron process

	Wsenseextdrv2n =  200 * FEATURESIZE;//this was 160 micron for the 0.8 micron process

	krise = krise / FUDGEFACTOR; //this was krise for the 0.8 micron process

	tsensedata = tsensedata / FUDGEFACTOR; //this was tsensedata for the 0.8 micron process

//#define psensedata      (0.02e-9)
//#define tsensescale     0.02e-10

	tsensetag = tsensetag / FUDGEFACTOR; //this was tsensetag for the 0.8 micron process

//#define psensetag	(0.016e-9)

	tfalldata = tfalldata / FUDGEFACTOR;//this was tfalldata for the 0.8 micron process
	tfalltag = tfalltag / FUDGEFACTOR;//this was tfalltag for the 0.8 micron process

	BitWidth = 7.746*FEATURESIZE;//this was 7.746*0.8 for the 0.8 micron process

	BitHeight = 2*7.746*FEATURESIZE;//this was 2*7.746*0.8 for the 0.8 micron process

	Cout = Cout/ FUDGEFACTOR;//this was 0.5e-12 for the 0.8 micron process


	//From areadef.h; expressing all dimensions in terms of process feauture size

  Widthptondiff = 4*FEATURESIZE; //Was 3.2 micron for the 0.8 micron process

  Widthtrack = 3.2*FEATURESIZE; //Was 3.2*0.8 micron for the 0.8 micron process

  Widthcontact = 2*FEATURESIZE; //Was 1.6 micron for the 0.8 micron process
  Wpoly = FEATURESIZE;//Was 0.8 micron for the 0.8 micron process

  ptocontact = FEATURESIZE/2; //Was 0.4 micron for the 0.8 micron process
  stitch_ramv = 7.5*FEATURESIZE; //Was 6 micron for the 0.8 micron process
//#define BitHeight16x2 33.6

  BitHeight1x1 = 2*7.746*FEATURESIZE; /* see below */ //Was 2*7.746*0.8 micron for the 0.8 micron process
  stitch_ramh = 15*FEATURESIZE;  //Was 12 micron for the 0.8 micron process
//#define BitWidth16x2  192.8
  BitWidth1x1 = 7.746*FEATURESIZE; //Was 7.746*0.8 micron for the 0.8 micron process
/* dt: Assume that each 6-T SRAM cell is 120F^2 and has an aspect ratio of 1(width) to 2(height), than the width is 2*sqrt(60)*F */
  WidthNOR1 = 14.5*FEATURESIZE; //Was 11.6 micron for the 0.8 micron process
  WidthNOR2 = 17*FEATURESIZE; //Was 13.6 micron for the 0.8 micron process
  WidthNOR3 = 26*FEATURESIZE;  //Was 20.8 micron for the 0.8 micron process
  WidthNOR4 = 36*FEATURESIZE;  //Was 28.8 micron for the 0.8 micron process
  WidthNOR5 = 43*FEATURESIZE;  //Was 34.4 micron for the 0.8 micron process
  WidthNOR6 = 52*FEATURESIZE; //Was 41.6 micron for the 0.8 micron process
  Predec_height1 = 176*FEATURESIZE; //Was 140.8 micron for the 0.8 micron process
  Predec_width1 = 338*FEATURESIZE;  //Was 270.4 micron for the 0.8 micron process
  Predec_height2 = 176*FEATURESIZE; //Was 140.8 micron for the 0.8 micron process
  Predec_width2 = 674*FEATURESIZE; //Was 539.2 micron for the 0.8 micron process
  Predec_height3 = 352*FEATURESIZE;  //Was 281.6 micron for the 0.8 micron process
  Predec_width3 = 730*FEATURESIZE; //Was 584 micron for the 0.8 micron process
  Predec_height4 = 352*FEATURESIZE; //Was 281.6 micron for the 0.8 micron process
  Predec_width4 = 786*FEATURESIZE; //Was 628.8 micron for the 0.8 micron process
  Predec_height5 = 528*FEATURESIZE; //Was 422.4 micron for the 0.8 micron process
  Predec_width5 = 842*FEATURESIZE; //Was 673.6 micron for the 0.8 micron process
  Predec_height6 = 528*FEATURESIZE; //Was 422.4 micron for the 0.8 micron process
  Predec_width6 = 898*FEATURESIZE; //Was 718.4 micron for the 0.8 micron process
  Wwrite = 1.5*FEATURESIZE; //Was 1.2 micron for the 0.8 micron process
  SenseampHeight = 190*FEATURESIZE; //Was 152 micron for the 0.8 micron process
  OutdriveHeight = 250*FEATURESIZE; //Was 200 micron for the 0.8 micron process
  FAOutdriveHeight = 286.5*FEATURESIZE; //Was 229.2 micron for the 0.8 micron process
  FArowWidth = 478.5*FEATURESIZE; //Was 382.8 micron for the 0.8 micron process
  CAM2x2Height_1p = 61*FEATURESIZE; //Was 48.8 micron for the 0.8 micron process
  CAM2x2Width_1p = 56*FEATURESIZE; //Was 44.8 micron for the 0.8 micron process
  CAM2x2Height_2p = 101*FEATURESIZE; //Was 80.8 micron for the 0.8 micron process
  CAM2x2Width_2p = 96*FEATURESIZE; //Was 76.8 micron for the 0.8 micron process
  DatainvHeight = 32*FEATURESIZE; //Was 25.6 micron for the 0.8 micron process
  Wbitdropv = 37.5*FEATURESIZE; //Was 30 micron for the 0.8 micron process
  decNandWidth = 43*FEATURESIZE; //Was 34.4 micron for the 0.8 micron process
  FArowNANDWidth = 89*FEATURESIZE; //Was 71.2 micron for the 0.8 micron process
  FArowNOR_INVWidth = 35*FEATURESIZE; //Was 28 micron for the 0.8 micron process


  FAHeightIncrPer_first_rw_or_w_port = 20*FEATURESIZE; //Was 16 micron for the 0.8 micron process
  FAHeightIncrPer_later_rw_or_w_port = 20*FEATURESIZE; //Was 16 micron for the 0.8 micron process
  FAHeightIncrPer_first_r_port = 15*FEATURESIZE; //Was 12 micron for the 0.8 micron process
  FAHeightIncrPer_later_r_port = 15*FEATURESIZE; //Was 12 micron for the 0.8 micron process
  FAWidthIncrPer_first_rw_or_w_port = 20*FEATURESIZE; //Was 16 micron for the 0.8 micron process
  FAWidthIncrPer_later_rw_or_w_port = 12*FEATURESIZE; //Was 9.6 micron for the 0.8 micron process
  FAWidthIncrPer_first_r_port = 15*FEATURESIZE; //Was 12 micron for the 0.8 micron process
  FAWidthIncrPer_later_r_port = 12*FEATURESIZE; //Was 9.6 micron for the 0.8 micron process



}
