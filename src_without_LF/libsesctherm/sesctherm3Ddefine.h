#ifndef  SESCTHERM3D_DEFINE_H
#define SESCTHERM3D_DEFINE_H

//#define _SESCTHERM_DEBUG 

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ?  (X) : (Y))
#endif
#define ODD(X) ((X) % 2 ?  (true) : (false))
#define EVEN(X) ((X) % 2 ?  (false) : (true))
#ifndef MAX
#define MAX(x,y)	(((x)>(y))?(x):(y))
#endif

#define DELTA		1.0e-9
#define ROUND(x)	(ceil(x*pow(10.0, 9.0))/pow(10.0,9.0))
#define EQ(x,y)		(fabs(x-y) <  DELTA)
#define LT(x,y)			(x<y && !(fabs(x-y) <  DELTA) )
#define GT(x,y)			(x>y && !(fabs(x-y) <  DELTA))
#define GE(x,y)			(x>y || (fabs(x-y) <  DELTA))
#define LE(x,y)			(x<y || (fabs(x-y) <  DELTA))
#define ABS_DIF(x,y)	((GE(x,y)) ? (fabs(x-y)) : (fabs(y-x)))

#define BOLTZMAN_CONST	5.67e-8

typedef double SUElement_t;

typedef matrix<SUElement_t,rectangle<>,compressed<>,column_major>::type Matrix;
typedef matrix<SUElement_t,rectangle<>,dense<>,column_major>::type MatrixDense;
typedef matrix_traits<matrix<SUElement_t,rectangle<>,compressed<>,row_major>::type>::size_type Matrix_itor;
//typedef matrix<SUElement_t,rectangle<>,compressed<>,column_major>::const_iterator Matrix_iter;
//typedef matrix<SUElement_t,rectangle<>,compressed<>,column_major>::OneD::const_iterator Matrix_iter_OneD;
typedef dense1D<SUElement_t> Vector;

//define the heat transfer types for the model (aka base) units
enum {HEAT_CONVECTION_TRANSFER, HEAT_CONDUCTION_TRANSFER, HEAT_RADIATION_TRANSFER};

//defines for either superlu library or mtl
enum {LIBRARY_SUPERLU, LIBRARY_MTL};

//define the technology
enum {TECH_250,TECH_180,TECH_130,TECH_90,TECH_65};


//The following defines the various kinds of materials
enum{ BULK_SI, SI_O2, POLYSILICON, COPPER, VIRTUAL, SIMOX, DIELECTRIC_PACKAGE, DIELECTRIC_CHIP};


//This defines the graphics file output
//			 [AVE_/DIF_][POWER/TEMP]_CUR_[M/F] -> output power/temperature for each timestep 
//			 [AVE_/DIF_][POWER/TEMP]_MAX_[M/F] -> output max power/temperature over each sample duration
//			 [AVE_/DIF_][POWER/TEMP]_MIN_[M/F] -> output min power/temperature over the sample duration
//			 [AVE_/DIF_][POWER/TEMP]_AVE_[M/F] -> output average power/temperature for sample duration 
enum{GFX_LAYER_FLOORPLAN, GFX_LAYER_NORMAL, GFX_LAYER_AVE, GFX_LAYER_DIF};			//nothing, compute average, compute difference across layers
enum{GFX_MUNIT, GFX_FUNIT};					//using model units/ using functional unit
enum{GFX_POWER, GFX_TEMP};					//use power values / use temperature values
enum{GFX_CUR, GFX_MAX, GFX_MIN, GFX_AVE};	//use cur for each timestep/ [max/min/ave] values for each sample




#define PINS_DYNARRAY_MINSIZE 200
#define PWB_DYNARRAY_MINSIZE 200
#define FCPBGA_DYNARRAY_MINSIZE 200
#define CHIP_DYNARRAY_MINSIZE 200
#define UCOOL_DYNARRAY_MINSIZE 200
#define	HEAT_SPREADER_DYNARRAY_MINSIZE 200
#define HEAT_SINK_DYNARRAY_MINSIZE 200
#define HEAT_SINK_FINS_DYNARRAY_MINSIZE 200
#define UNSOLVED_MATRIX_DYNARRAY_MINSIZE 4000
enum{ MAINBOARD_LAYER, PINS_LAYER, PWB_LAYER, FCPBGA_LAYER, C4_UNDERFILL_LAYER, INTERCONNECT_LAYER, DIE_TRANSISTOR_LAYER, BULK_SI_LAYER, OIL_LAYER, AIR_LAYER, UCOOL_LAYER, HEAT_SPREADER_LAYER, HEAT_SINK_LAYER, HEAT_SINK_FINS_LAYER};
enum{ BASIC_EQUATION, UCOOL_COLD_EQUATION, UCOOL_HOT_EQUATION, UCOOL_INNER_LAYER_EQUATION};


//parameters to calculate polysilicon thermal conductivity at 300K
#define PHOTON_GROUP_VELOCITY 6166	// (m/s)
#define PHOTON_SPECIFIC_HEAT 1.654e6 // (J/m^3*K)

#define TECH_PROCESS			0
#define SRAM_CELL				TECH_PROCESS+1
#define TECH_VDD				SRAM_CELL+1
#define TECH_GATE_L_AVE			TECH_VDD+1
#define TECH_GATE_PMOS_W_AVE	TECH_GATE_L_AVE+1
#define TECH_GATE_NMOS_W_AVE	TECH_GATE_PMOS_W_AVE+1
#define TECH_LEFF				TECH_GATE_NMOS_W_AVE+1
#define TECH_TOX				TECH_LEFF+1
#define	TECH_LEVELS				TECH_TOX+1
#define TECH_K					TECH_LEVELS+1
#define TECH_POLY				10
#define TECH_METAL1				TECH_POLY+9
#define TECH_METAL2				TECH_METAL1+9
#define TECH_METAL3				TECH_METAL2+9
#define TECH_METAL4				TECH_METAL3+9
#define TECH_METAL5				TECH_METAL4+9
#define TECH_METAL6				TECH_METAL5+9
#define TECH_METAL7				TECH_METAL6+9
#define TECH_METAL8				TECH_METAL7+9
//#define TECH_METAL9		78 FIXME: Metal 9 data is completely wrong here (45nm process)



enum{TECH_H, TECH_W, TECH_SPACE, TECH_RESIST, TECH_CAP, TECH_SOPT, TECH_TINS, TECH_VIADIST, TECH_NUMWIRES};

//These are used for the effective RC computation
enum{RC_CHARGING, RC_DISCHARGING};
#endif
