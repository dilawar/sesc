

#include "sesctherm3Dinclude.h"

//const int32_t sesctherm3Ddatalibrary::gate_pitches[5]={1932,2760,3900,5571,7959};



int sesctherm3Ddatalibrary::tech_metal_index(int layer){
	return(TECH_POLY+9*layer);
}

/* FIXME: this should be put in a separate file (and integrated with SESC, not possible with sesctherm)
const double datalibrary::leakage_technology_parameters_[25][4] = {
	{180,	130,	100,	70},						//Process lithography (nm)
	{3.5E-9,	3.3E-9,	2.5E-9,	1.0E-9},				//oxide thickness
	{2.0,		1.5,	1.0,	0.9},					//ireg_voltage
	{ 0.3979, 0.3353, 0.2607, 0.3979},					//ireg_threshold_Cell_N
	{0.4659, 0.3499,0.3030, 0.2130},					//ireg_threshold_Access_N
	{2.0,1.5,1.0,2.0},									//il1_voltage
	{2.0,1.5,1.0,2.0},									//il2_voltage
	{2.0,1.5,1.0,2.0},									//dl1_voltage
	{2.0,1.5,1.0,2.0},									//dl2_voltage
	{0.3979,0.3353,0.2607,0.1902},						//il1_threshold_Cell_N
	{0.4659,0.3499,0.3030,0.2130},						//il1_threshold_Cell_P
	{0.3979,0.3353,0.2607,0.1902},						//il1_threshold_Access_N
	{0.3979,0.3353,0.2607,0.1902},						//il2_threshold_Cell_N
	{0.4659,0.3499,0.3030,0.2130},						//il2_threshold_Cell_P
	{0.3979,0.3353,0.2607,0.1902},						//il2_threshold_Access_N
	
	{0.4659,0.3499,0.3030,0.2130},						//il1_threshold_P
	{0.4659,0.3499,0.3030,0.2130},						//il2_threshold_P
	{0.3979,0.3353,0.2607,0.1902},						//dl1_threshold_Cell_N
	{0.4659,0.3499,0.3030,0.2130},						//dl1_threshold_Cell_P
	{0.3979,0.3353,0.2607,0.1902},						//dl1_threshold_Access_N 
	{0.4659,0.3499,0.3030,0.2130},						//dl2_threshold_P 
	{0.4659,0.3499,0.3030,0.2130},						//dl1_threshold_P
	{0.3979,0.3353,0.2607,0.1902},						//dl2_threshold_Cell_N
	{0.4659,0.3499,0.3030,0.2130},						//dl2_threshold_Cell_P 
	{0.3979,0.3353,0.2607,0.1902}						//dl2_threshold_Access_N 
}

*/


sesctherm3Ddatalibrary::sesctherm3Ddatalibrary(){
	config_data_= new config_data(this);				
	graphics_= new sesctherm3Dgraphics(this);
	//unsolved_matrix_dyn_ =	new dynamic_array <double> (UNSOLVED_MATRIX_DYNARRAY_MINSIZE,UNSOLVED_MATRIX_DYNARRAY_MINSIZE, this);
	rhs_size_superlu_=-1;
	rhs_superlu_=NULL;
	previous_temperature_matrix_=NULL;
	non_zero_vals_=NULL;
	row_indices_=NULL;
	col_begin_indices_=NULL;
	accumulate_rc_=false;
	use_rc_=false;
}

sesctherm3Ddatalibrary::~sesctherm3Ddatalibrary(){
  delete LU_mtl_;
  delete pvector_mtl_;
#ifdef USE_MP
  Destroy_SuperNode_SCP(&L_superlu_);
  Destroy_CompCol_NCP(&U_superlu_);
#else
  Destroy_SuperMatrix_Store(&L_superlu_); 
  Destroy_CompCol_Permuted(&U_superlu_); 
#endif
  delete config_data_;
	//delete unsolved_matrix_dyn_;
}





//FIXME: add 45nm parameters
//FIXME: add #wires for 250,180nm and poly layers
//FIXME: add #wires for INTEL/TSMC (only IBM current used, see technology parameters reference)
//FIXME: #wires should be scaled using sesctherm3Dinterconnect.cpp
const double sesctherm3Ddatalibrary::technology_parameters_[91][5] = {
	{250e-9,	180e-9,	130e-9,	90e-9,		65e-9},				//Process lithography (nm)
	{4643e-9,	3083e-9,	2000e-9,	1000e-9,	570e-9},	//6-T SRAM Cell Size (nm)			
	{2,		1.8,	1.5,	1.057,	0.884},						//Vdd (V)
	{500e-9,	360e-9,	260e-9,	180e-9,	130e-9},				//Gate length of average-sized nmos/pmos (nm) (2xminimum)
	{14280e-9,	10280e-9,	7420e-9,	5130e-9,	3700e-9},	//Gate width of average-sized pmos (2xminimum) 			
	{5720e-9,	412e-9,	298e-9,	207e-9,	150e-9},				//Gate width of average-sized nmos (2xminimum)
	{160,	100,	70,		46.571,	29.844},					//Leff (nm)
	{60,	45,		35,		24.472,	18.7260},					//tox(Angstroms = 1.0x10-10 meters)
	{6,		6,		7,		8,		9},							//levels (metal layers, includes M0 (POLY) )

	//interlayer-dielectric constant k_ild
	//Taken from (1) Thermal Scaling analysis of multilevel cu/low-k interconnect structures
	// (2) A 90nm Logic technology featuring 50nm strained silicon channel transistors, ...
	// (3) An enhanced 130nm generation logic technology featuring 60nm transistors optimied for high performance and...	
	
	{5.81,	4.53,	3.6,	2.9,	2.4},						//k_ild
	
	//Poly
	{289e-9 , 217e-9 , 160e-9 , 140e-9, 90e-9},										//H				
	{502e-9,	395e-9,	319e-9,	260e-9,	220e-9},									//W			
	{502e-9,	395e-9,	319e-9,	260e-9,	220e-9},									//Space			
	{1589.688171,	1547.624512,	393.9827542,	688.7825641,	839.7005831},	//wire resistance	
	{0.140922039,	0.275407057,	0.317581512,	0.323601485,	0.327612261},	//wire capacitance	
	{4.564777016,	2.922899361,	6.107996587,	4.34055825,		3.722697363},	//sopt	
	{322e-9,	235e-9,	97e-9,	72e-9,	30e-9},										//tins				
	{400e-9,	260e-9,	140e-9,	100e-9,	70e-9},										//inter-via distances		
	{0,		0,		0,		0,	0},													//number of wires	
	
	
	//Metal 1
	{389e-9,	304e-9,	280e-9,	150e-9,	170e-9},									//H	
	{445e-9,	353e-9,	293e-9,	220e-9,	210e-9},									//W	
	{445e-9,	353e-9,	293e-9,	220e-9,	210e-9},									//Space	
	{1540.715952,	1500.414608,	384.6548897,	669.1671877,	682.068109},	//wire resistance
	{0.14493629,	0.279517033,	0.319996725,	0.325661893,	0.329137069},	//wire capacitance
	{4.572093864,	2.946621908,	6.158239884,	4.389763505,	4.120953955},	//sopt
	{434e-9,	329e-9,	169e-9,	77e-9,	57e-9},										//tins	
	{299e-9,	186e-9,	64e-9,		63e-9,		63e-9},								//inter-via distances		
	{0,		0,26250594,	51954488,0},												//number of wires
	
	//Metal 2
	{467e-9,	378e-9,	360e-9,	256e-9,	190e-9},									//H	
	{760e-9,	560e-9,	425e-9,	320e-9,	210e-9},									//W	
	{760e-9,	560e-9,	425e-9,	320e-9,	210e-9},									//Space	
	{1540.715952,	1500.414608,	384.6548897,	669.1671877,	682.068109},	//wire resistance
	{0.14493629,	0.279517033,	0.319996725,	0.325661893,	0.329137069},	//wire capacitance
	{4.572093864,	2.946621908,	6.158239884,	4.389763505,	4.120953955},	//sopt
	{521e-9,	409e-9,	217e-9,	131e-9,	64e-9},										//tins
	{1498e-9,	968e-9,	442e-9,	196e-9,	63e-9},										//inter-via distances
	{0,		0,	1995879,	4629072,0},												//number of wires
	
	//Metal 3
	{460e-9,	375e-9,	360e-9,	256e-9,	200e-9},									//H	
	{743e-9,	552e-9,	425e-9,	320e-9,	220e-9},									//W	
	{743e-9,	552e-9,	425e-9,	320e-9,	220e-9},									//Space	
	{107,	107,	188,	414.5524582,	627.9467305},							//wire resistance
	{0.202,	0.333,	0.336,	0.335053232,	0.335761407},							//wire capacitance
	{14.69594182,	10.10927913,	8.596396413,	5.49851306,	4.252293599},		//sopt
	{513e-9,	406e-9,	217e-9,	131e-9,	67e-9},										//tins
	{1373e-9,	908e-9,	442e-9,	196e-9,	70e-9},										//inter-via distances
	{0,		0,	572524,	1270748,0},													//number of wires
	
	
	//Metal 4
	{766e-9,	592e-9,	570e-9,	320e-9,	250e-9},									//H	
	{1368e-9,	961e-9,	718e-9,	400e-9,	280e-9},									//W
	{1368e-9,	961e-9,	718e-9,	400e-9,	280e-9},									//Space	
	{107,	107,	188,	414.5524582,	627.9467305},							//wire resistance
	{0.202,	0.333,	0.336,	0.335053232,	0.335761407},							//wire capacitance
	{14.69594182,	10.10927913,	8.596396413,	5.49851306,	4.252293599},		//sopt
	{854e-9,	640e-9,	344e-9,	164e-9,	84e-9},										//tins
	{33628e-9,	23606e-9,	10573e-9,	482e-9,	136e-9},							//inter-via distance			
	{0,		0,	208671,	456388,0},													//number of wires
	
	
	//Metal 5
	{1271e-9,	935e-9,	900e-9,	384e-9,	300e-9},									//H	
	{2167e-9,	1470e-9,	1064e-9,	480e-9,	330e-9},							//W			
	{2167e-9,	1470e-9,	1064e-9,	480e-9,	330e-9},							//space			
	{161.2217667,	160.5662844,	68.29904873,	83.2515535,	187.3168749},		//wire resistance
	{0.193670237,	0.325685993,	0.346020375,	0.347706972,	0.348226762},	//wire capacitance
	{12.22705083,	8.344637493,	14.0542228,	12.04450737,	7.645053143},		//sopt
	{1416e-9,	1012e-9,	543e-9,	197e-9,	101e-9},								//tins
	{2006481e-9,	1361111e-9,	449548e-9,	1184e-9,	238e-9},					//inter-via distance
	{0,		0,	85245,		188980,0},												//number of wires
	
	//Metal 6
	{0e-9,	0e-9,	1200e-9,	576e-9,	430e-9},									//H
	{0e-9,	0e-9,	1143e-9,	720e-9,	480e-9},									//W
	{0e-9,	0e-9,	1143e-9,	720e-9,	480e-9},									//space	
	{0,	0,	68.29904873,	83.2515535,	187.3168749},								//wire resistance
	{0,	0,	0.346020375,	0.347706972,	0.348226762},							//wire capacitance
	{0,	0,	14.0542228,	12.04450737,	7.645053143},								//sopt
	{0e-9,	0e-9,	724e-9,	295e-9,	145e-9},										//tins
	{0e-9,	0e-9,	1058333e-9,	17535e-9,	1262e-9},								//inter-via distance
	{0,	0,	36318,	49918,0},														//number of wires
	
	//Metal 7
	{0e-9,	0e-9,	0e-9,	972e-9,	650e-9},										//H
	{0e-9,	0e-9,	0e-9,	1080e-9,	720e-9},									//W
	{0e-9,	0e-9,	0e-9,	1080e-9,	720e-9},									//space
	{0,	0,	0,	4.687560857,	18.20896974},										//wire resistance
	{0,	0,	0,	0.357893674,	0.358988515},										//wire capacitance
	{0,	0,	0,	50.03126038,	24.14999628},										//sopt
	{0e-9,	0e-9,	0e-9,	498e-9,	219e-9},										//tins
	{0e-9,	0e-9,	0e-9,	1000000e-9,18228e-9},									//inter-via distance
	{0,	0,	0,	60162,0},															//number of wires
	
	//Metal 8
	{0e-9,	0e-9,	0e-9,	0e-9,	975e-9},										//H
	{0e-9,	0e-9,	0e-9,	0e-9,	1080e-9},										//W
	{0e-9,	0e-9,	0e-9,	0e-9,	1080e-9},										//space
	{0,	0,	0,	0,	18.20896974},													//wire resistance
	{0,	0,	0,	0,	0.358988515},													//wire capacitance
	{0e-9,	0e-9,	0e-9,	0e-9,	24.14999628e-9},								//sopt
	{0e-9,	0e-9,	0e-9,	0e-9,	329e-9},										//tins
	{0,	0,	0,	0,	1000000},														//inter-via distance
	{0, 0,  0,  0,  0}																//number of wires
};	



/* Most of the technology parameters were taken from:
"Investigation of Performance Metrics For DSM Interconnect"
http://www.eecs.umich.edu/~kimyz/interconnect.htm

65nm gate pitch = 1932nm
90nm gate pitch = 2760nm
130nm gate pitch = 3900nm
180nm gate pitch = 5571nm
250nm gate pitch = 7959nm
This represents the average gate-to-gate distance (not to be confused with transitor-to-transistor distance)

Krishnan et al and Vikas et al were used


Further data values were taken from:'Accurate Energy Dissipation and Thermal Modeling for Nanometer-Scale Buses', Krishnan Sundaresan and Nihar R. Mahapatra
Further data values were taken from: The Effect of Technology Scaling on Microarchitectural Structures, Vikas Agarwal, Stephen W. Keckler, Doug Burger


* lcrit and sopt values for intermediate layer
* are derived from the values for global layer.
* lcrit * sqrt(wire_r * wire_c) is a constant
* for a given layer. So is the expression
* sopt * sqrt(wire_r / wire_c) (Equation 2 and
								* Theorem 2 from Brayton et. al). Using this
* for the global layer, one can find the constants
* for the intermediate layer and combining this
* with the wire_r and wire_c values for the
* intermediate layer, its lcrit and sopt values
* can be calculated.



*	We interpolate the data values for the wire resistances and the capacitances for each layer.
*	This is done assuming that the wire ressitances are proportional to the cross-sectional area of the wire.
*	Further, we assume that the wire capacitance is a function of the height of the wire
*
*/




