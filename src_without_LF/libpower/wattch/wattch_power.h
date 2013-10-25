/*------------------------------------------------------------
 *  Copyright 1994 Digital Equipment Corporation and Steve Wilton
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein,
 * and hereby grant back to Digital a non-exclusive, unrestricted, royalty-
 * free right and license under any changes, enhancements or extensions
 * made to the core functions of the software, including but not limited to
 * those affording compatibility with other hardware or software
 * environments, but excluding applications which incorporate this software.
 * Users further agree to use their best efforts to return to Digital any
 * such changes, enhancements or extensions that they make and inform Digital
 * of noteworthy uses of this software.  Correspondence should be provided
 * to Digital at:
 *
 *                       Director of Licensing
 *                       Western Research Laboratory
 *                       Digital Equipment Corporation
 *                       100 Hamilton Avenue
 *                       Palo Alto, California  94301
 *
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *------------------------------------------------------------*/


/*  The following are things you might want to change
 *  when compiling
 */

/*
 * The output can be in 'long' format, which shows everything, or
 * 'short' format, which is just what a program like 'grap' would
 * want to see
 */
#ifndef POWER_H
#define POWER_H

#include "sys/types.h"
#include "setup.h"
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LONG 1
#define SHORT 2

#define OUTPUTTYPE LONG

/* Do we want static AFs (STATIC_AF) or Dynamic AFs (DYNAMIC_AF) */
/* #define DYNAMIC_AF */
#define DYNAMIC_AF

/*
 * Address bits in a word, and number of output bits from the cache 
 */

#define ADDRESS_BITS 64
#define BITOUT 64

/* limits on the various N parameters */

#define MAXN 8            /* Maximum for Ndwl,Ntwl,Ndbl,Ntbl */
#define MAXSUBARRAYS 8    /* Maximum subarrays for data and tag arrays */
#define MAXSPD 8          /* Maximum for Nspd, Ntspd */


/*===================================================================*/

/*
 * The following are things you probably wouldn't want to change.  
 */


#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define BIGNUM 1e30
#define DIVIDE(a,b) ((b)==0)? 0:(a)/(b)
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* Used to communicate with the horowitz model */

#define RISE 1
#define FALL 0
#define NCH  1
#define PCH  0

/*
 * The following scale factor can be used to scale between technologies.
 * To convert from 0.8um to 0.5um, make FUDGEFACTOR = 1.6
 */
#define FUDGEFACTOR 1
 

/*===================================================================*/

/*
 * Cache layout parameters and process parameters 
 * Thanks to Glenn Reinman for the technology scaling factors
 */

#define GEN_POWER_FACTOR 1.31

#define TECH_POINT07
#if defined(TECH_POINT07)
#define CSCALE		(172.2157)	/* wire capacitance scaling factor */
#define RSCALE		(160.0000)	/* wire resistance scaling factor */
#define LSCALE		0.0875		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.36		/* voltage scaling factor */
#define VTSCALE		0.46		/* threshold voltage scaling factor */
#define SSCALE		0.75		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (1/(GEN_POWER_FACTOR*GEN_POWER_FACTOR))
#undef FUDGEFACTOR
#define FUDGEFACTOR 11.4

#elif defined(TECH_POINT10)
#define CSCALE		(84.2172)	/* wire capacitance scaling factor */
			/* linear: 51.7172, predicted: 84.2172 */
#define RSCALE		(80.0000)	/* wire resistance scaling factor */
#define LSCALE		0.1250		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.38		/* voltage scaling factor */
#define VTSCALE		0.49		/* threshold voltage scaling factor */
#define SSCALE		0.80		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (1/GEN_POWER_FACTOR)
#elif defined(TECH_POINT18)
#define CSCALE		(19.7172)	/* wire capacitance scaling factor */
#define RSCALE		(20.0000)	/* wire resistance scaling factor */
#define LSCALE		0.2250		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.4		/* voltage scaling factor */
#define VTSCALE		0.5046		/* threshold voltage scaling factor */
#define SSCALE		0.85		/* sense voltage scaling factor */
#define GEN_POWER_SCALE 1
#elif defined(TECH_POINT25)
#define CSCALE		(10.2197)	/* wire capacitance scaling factor */
#define RSCALE		(10.2571)	/* wire resistance scaling factor */
#define LSCALE		0.3571		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.45		/* voltage scaling factor */
#define VTSCALE		0.5596		/* threshold voltage scaling factor */
#define SSCALE		0.90		/* sense voltage scaling factor */
#define GEN_POWER_SCALE GEN_POWER_FACTOR
#elif defined(TECH_POINT35a)
#define CSCALE		(5.2197)	/* wire capacitance scaling factor */
#define RSCALE		(5.2571)	/* wire resistance scaling factor */
#define LSCALE		0.4375		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.5		/* voltage scaling factor */
#define VTSCALE		0.6147		/* threshold voltage scaling factor */
#define SSCALE		0.95		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (GEN_POWER_FACTOR*GEN_POWER_FACTOR)
#elif defined(TECH_POINT35)
#define CSCALE		(5.2197)	/* wire capacitance scaling factor */
#define RSCALE		(5.2571)	/* wire resistance scaling factor */
#define LSCALE		0.4375		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.5		/* voltage scaling factor */
#define VTSCALE		0.6147		/* threshold voltage scaling factor */
#define SSCALE		0.95		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (GEN_POWER_FACTOR*GEN_POWER_FACTOR)
#elif defined(TECH_POINT40)
#define CSCALE		1.0		/* wire capacitance scaling factor */
#define RSCALE		1.0		/* wire resistance scaling factor */
#define LSCALE		0.5		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		1.0		/* voltage scaling factor */
#define VTSCALE		1.0		/* threshold voltage scaling factor */
#define SSCALE		1.0		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR)
#else /* TECH_POINT80 */
/* scaling factors */
#define CSCALE		1.0		/* wire capacitance scaling factor */
#define RSCALE		1.0		/* wire resistance scaling factor */
#define LSCALE		1.0		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		1.0		/* voltage scaling factor */
#define VTSCALE		1.0		/* threshold voltage scaling factor */
#define SSCALE		1.0		/* sense voltage scaling factor */
#define GEN_POWER_SCALE (GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR)
#endif

/*
 * CMOS 0.8um model parameters
 *   - from Appendix II of Cacti tech report
 */


/* corresponds to 8um of m3 @ 225ff/um */
#define Cwordmetal    (1.8e-15 * (CSCALE * ASCALE))

/* corresponds to 16um of m2 @ 275ff/um */
#define Cbitmetal     (4.4e-15 * (CSCALE * ASCALE))

/* corresponds to 1um of m2 @ 275ff/um */
#define Cmetal        Cbitmetal/16 

#define CM3metal        Cbitmetal/16 
#define CM2metal        Cbitmetal/16 

/* #define Cmetal 1.222e-15 */

/* fF/um2 at 1.5V */
#define Cndiffarea    0.137e-15		/* FIXME: ??? */

/* fF/um2 at 1.5V */
#define Cpdiffarea    0.343e-15		/* FIXME: ??? */

/* fF/um at 1.5V */
#define Cndiffside    0.275e-15		/* in general this does not scale */

/* fF/um at 1.5V */
#define Cpdiffside    0.275e-15		/* in general this does not scale */

/* fF/um at 1.5V */
#define Cndiffovlp    0.138e-15		/* FIXME: by depth??? */

/* fF/um at 1.5V */
#define Cpdiffovlp    0.138e-15		/* FIXME: by depth??? */

/* fF/um assuming 25% Miller effect */
#define Cnoxideovlp   0.263e-15		/* FIXME: by depth??? */

/* fF/um assuming 25% Miller effect */
#define Cpoxideovlp   0.338e-15		/* FIXME: by depth??? */

/* um */
#define Leff          (0.8 * LSCALE)

/* fF/um2 */
#define Cgate         1.95e-15		/* FIXME: ??? */

/* fF/um2 */
#define Cgatepass     1.45e-15		/* FIXME: ??? */

/* note that the value of Cgatepass will be different depending on 
   whether or not the source and drain are at different potentials or
   the same potential.  The two values were averaged */

/* fF/um */
#define Cpolywire	(0.25e-15 * CSCALE * LSCALE)

/* ohms*um of channel width */
#define Rnchannelstatic	(25800 * LSCALE)

/* ohms*um of channel width */
#define Rpchannelstatic	(61200 * LSCALE)

#define Rnchannelon	(9723 * LSCALE)

#define Rpchannelon	(22400 * LSCALE)

/* corresponds to 16um of m2 @ 48mO/sq */
#define Rbitmetal	(0.320 * (RSCALE * ASCALE))

/* corresponds to  8um of m3 @ 24mO/sq */
#define Rwordmetal	(0.080 * (RSCALE * ASCALE))

#define Vdd		(5 * VSCALE)

/* other stuff (from tech report, appendix 1) */

// #define Mhz             1000e6

#define Period          (1/Mhz)

#define krise		(0.4e-9 * LSCALE)
#define tsensedata	(5.8e-10 * LSCALE)
#define tsensetag	(2.6e-10 * LSCALE)
#define tfalldata	(7e-10 * LSCALE)
#define tfalltag	(7e-10 * LSCALE)
#define Vbitpre		(3.3 * SSCALE)
#define Vt		(1.09 * VTSCALE)
#define Vbitsense	(0.10 * SSCALE)

#define Powerfactor (Mhz)*Vdd*Vdd

#define SensePowerfactor3 (Mhz)*(Vbitsense)*(Vbitsense)
#define SensePowerfactor2 (Mhz)*(Vbitpre-Vbitsense)*(Vbitpre-Vbitsense)
#define SensePowerfactor (Mhz)*(Vdd/2)*(Vdd/2)

#define AF    .5
#define POPCOUNT_AF  (23.9/64.0)

/* Threshold voltages (as a proportion of Vdd)
   If you don't know them, set all values to 0.5 */

#define VSINV         0.456   
#define VTHINV100x60  0.438   /* inverter with p=100,n=60 */
#define VTHNAND60x90  0.561   /* nand with p=60 and three n=90 */
#define VTHNOR12x4x1  0.503   /* nor with p=12, n=4, 1 input */
#define VTHNOR12x4x2  0.452   /* nor with p=12, n=4, 2 inputs */
#define VTHNOR12x4x3  0.417   /* nor with p=12, n=4, 3 inputs */
#define VTHNOR12x4x4  0.390   /* nor with p=12, n=4, 4 inputs */
#define VTHOUTDRINV    0.437
#define VTHOUTDRNOR   0.431
#define VTHOUTDRNAND  0.441
#define VTHOUTDRIVE   0.425
#define VTHCOMPINV    0.437
#define VTHMUXDRV1    0.437
#define VTHMUXDRV2    0.486
#define VTHMUXDRV3    0.437
#define VTHEVALINV    0.267
#define VTHSENSEEXTDRV  0.437

/* transistor widths in um (as described in tech report, appendix 1) */
#define Wdecdrivep	(57.0 * LSCALE)
#define Wdecdriven	(40.0 * LSCALE)
#define Wdec3to8n	(14.4 * LSCALE)
#define Wdec3to8p	(14.4 * LSCALE)
#define WdecNORn	(5.4 * LSCALE)
#define WdecNORp	(30.5 * LSCALE)
#define Wdecinvn	(5.0 * LSCALE)
#define Wdecinvp	(10.0  * LSCALE)

#define Wworddrivemax	(100.0 * LSCALE)
#define Wmemcella	(2.4 * LSCALE)
#define Wmemcellr	(4.0 * LSCALE)
#define Wmemcellw	(2.1 * LSCALE)
#define Wmemcellbscale	2		/* means 2x bigger than Wmemcella */
#define Wbitpreequ	(10.0 * LSCALE)

#define Wbitmuxn	(10.0 * LSCALE)
#define WsenseQ1to4	(4.0 * LSCALE)
#define Wcompinvp1	(10.0 * LSCALE)
#define Wcompinvn1	(6.0 * LSCALE)
#define Wcompinvp2	(20.0 * LSCALE)
#define Wcompinvn2	(12.0 * LSCALE)
#define Wcompinvp3	(40.0 * LSCALE)
#define Wcompinvn3	(24.0 * LSCALE)
#define Wevalinvp	(20.0 * LSCALE)
#define Wevalinvn	(80.0 * LSCALE)

#define Wcompn		(20.0 * LSCALE)
#define Wcompp		(30.0 * LSCALE)
#define Wcomppreequ     (40.0 * LSCALE)
#define Wmuxdrv12n	(30.0 * LSCALE)
#define Wmuxdrv12p	(50.0 * LSCALE)
#define WmuxdrvNANDn    (20.0 * LSCALE)
#define WmuxdrvNANDp    (80.0 * LSCALE)
#define WmuxdrvNORn	(60.0 * LSCALE)
#define WmuxdrvNORp	(80.0 * LSCALE)
#define Wmuxdrv3n	(200.0 * LSCALE)
#define Wmuxdrv3p	(480.0 * LSCALE)
#define Woutdrvseln	(12.0 * LSCALE)
#define Woutdrvselp	(20.0 * LSCALE)
#define Woutdrvnandn	(24.0 * LSCALE)
#define Woutdrvnandp	(10.0 * LSCALE)
#define Woutdrvnorn	(6.0 * LSCALE)
#define Woutdrvnorp	(40.0 * LSCALE)
#define Woutdrivern	(48.0 * LSCALE)
#define Woutdriverp	(80.0 * LSCALE)

#define Wcompcellpd2    (2.4 * LSCALE)
#define Wcompdrivern    (400.0 * LSCALE)
#define Wcompdriverp    (800.0 * LSCALE)
#define Wcomparen2      (40.0 * LSCALE)
#define Wcomparen1      (20.0 * LSCALE)
#define Wmatchpchg      (10.0 * LSCALE)
#define Wmatchinvn      (10.0 * LSCALE)
#define Wmatchinvp      (20.0 * LSCALE)
#define Wmatchnandn     (20.0 * LSCALE)
#define Wmatchnandp     (10.0 * LSCALE)
#define Wmatchnorn     (20.0 * LSCALE)
#define Wmatchnorp     (10.0 * LSCALE)

#define WSelORn         (10.0 * LSCALE)
#define WSelORprequ     (40.0 * LSCALE)
#define WSelPn          (10.0 * LSCALE)
#define WSelPp          (15.0 * LSCALE)
#define WSelEnn         (5.0 * LSCALE)
#define WSelEnp         (10.0 * LSCALE)

#define Wsenseextdrv1p  (40.0*LSCALE)
#define Wsenseextdrv1n  (24.0*LSCALE)
#define Wsenseextdrv2p  (200.0*LSCALE)
#define Wsenseextdrv2n  (120.0*LSCALE)


/* bit width of RAM cell in um */
#define BitWidth	(16.0 * LSCALE)

/* bit height of RAM cell in um */
#define BitHeight	(16.0 * LSCALE)

#define Cout		(0.5e-12 * LSCALE)

/* Sizing of cells and spacings */
#define RatCellHeight    (40.0 * LSCALE)
#define RatCellWidth     (70.0 * LSCALE)
#define RatShiftRegWidth (120.0 * LSCALE)
#define RatNumShift      4
#define BitlineSpacing   (6.0 * LSCALE)
#define WordlineSpacing  (6.0 * LSCALE)

#define RegCellHeight    (16.0 * LSCALE)
#define RegCellWidth     (8.0  * LSCALE)

#define CamCellHeight    (40.0 * LSCALE)
#define CamCellWidth     (25.0 * LSCALE)
#define MatchlineSpacing (6.0 * LSCALE)
#define TaglineSpacing   (6.0 * LSCALE)

/*===================================================================*/

/* ALU POWER NUMBERS for .18um 733Mhz */
/* normalize to cap from W */
#define NORMALIZE_SCALE (1.0/(733.0e6*1.45*1.45))
/* normalize .18um cap to other gen's cap, then xPowerfactor */
#define POWER_SCALE    (GEN_POWER_SCALE * NORMALIZE_SCALE * Powerfactor)
#define I_ADD          ((.37 - .091) * POWER_SCALE)
#define I_ADD32        (((.37 - .091)/2)*POWER_SCALE)
#define I_MULT16       ((.31-.095)*POWER_SCALE)
#define I_SHIFT        ((.21-.089)*POWER_SCALE)
#define I_LOGIC        ((.04-.015)*POWER_SCALE)
#define F_ADD          ((1.307-.452)*POWER_SCALE)
#define F_MULT         ((1.307-.452)*POWER_SCALE)

#define I_ADD_CLOCK    (.091*POWER_SCALE)
#define I_MULT_CLOCK   (.095*POWER_SCALE)
#define I_SHIFT_CLOCK  (.089*POWER_SCALE)
#define I_LOGIC_CLOCK  (.015*POWER_SCALE)
#define F_ADD_CLOCK    (.452*POWER_SCALE)
#define F_MULT_CLOCK   (.452*POWER_SCALE)


/* Used to pass values around the program */

typedef struct {
   int tech;
   int iw;
   int winsize;
   int nvreg;
   int npreg;
   int nvreg_width;
   int npreg_width;
   int data_width;
} parameter_type;

typedef struct {
  double btb;
  double local_predict;
  double global_predict;
  double chooser;
  double ras;
  double rat_driver;
  double rat_decoder;
  double rat_wordline;
  double rat_bitline;
  double rat_senseamp;
  double dcl_compare;
  double dcl_pencode;
  double inst_decoder_power;
  double wakeup_tagdrive;
  double wakeup_tagmatch;
  double wakeup_ormatch;
  double ldq_wakeup_tagdrive;
  double ldq_wakeup_tagmatch;
  double ldq_wakeup_ormatch;
  double stq_wakeup_tagdrive;
  double stq_wakeup_tagmatch;
  double stq_wakeup_ormatch;
  double selection;
  double regfile_driver;
  double regfile_decoder;
  double regfile_wordline;
  double regfile_bitline;
  double regfile_senseamp;
  double reorder_driver;
  double reorder_decoder;
  double reorder_wordline;
  double reorder_bitline;
  double reorder_senseamp;
  double rs_driver;
  double rs_decoder;
  double rs_wordline;
  double rs_bitline;
  double rs_senseamp;
  double ldq_rs_driver;
  double ldq_rs_decoder;
  double ldq_rs_wordline;
  double ldq_rs_bitline;
  double ldq_rs_senseamp;

  double stq_rs_driver;
  double stq_rs_decoder;
  double stq_rs_wordline;
  double stq_rs_bitline;
  double stq_rs_senseamp;

  double resultbus;

  double icache_decoder;
  double icache_wordline;
  double icache_bitline;
  double icache_senseamp;
  double icache_tagarray;

  double icache;

  double dcache_decoder;
  double dcache_wordline;
  double dcache_bitline;
  double dcache_senseamp;
  double dcache_tagarray;

  double dtlb;
  double itlb;

  double dcache2_decoder;
  double dcache2_wordline;
  double dcache2_bitline;
  double dcache2_senseamp;
  double dcache2_tagarray;

  double total_power;
  double total_power_nodcache2;
  double ialu_power;
  double falu_power;
  double bpred_power;
  double rename_power;
  double rat_power;
  double dcl_power;
  double window_power;
  double ldq_power;
  double stq_power;
  double wakeup_power;
  double ldq_wakeup_power;
  double stq_wakeup_power;
  double rs_power;
  double rs_power_nobit;
  double ldq_rs_power;
  double stq_rs_power;
  double ldq_rs_power_nobit;
  double stq_rs_power_nobit;
  double selection_power;
  double regfile_power;
  double regfile_power_nobit;
  double result_power;
  double icache_power;
  double dcache_power;
  double dcache2_power;
  double reorder_power;

  double clock_power;

} power_result_type;

/* Used to pass values around the program */

typedef struct {
   int cache_size;
   int number_of_sets;
   int associativity;
   int block_size;
} time_parameter_type;

typedef struct {
   double access_time,cycle_time;
   int best_Ndwl,best_Ndbl;
   int best_Nspd;
   int best_Ntwl,best_Ntbl;
   int best_Ntspd;
   double decoder_delay_data,decoder_delay_tag;
   double dec_data_driver,dec_data_3to8,dec_data_inv;
   double dec_tag_driver,dec_tag_3to8,dec_tag_inv;
   double wordline_delay_data,wordline_delay_tag;
   double bitline_delay_data,bitline_delay_tag;
  double sense_amp_delay_data,sense_amp_delay_tag;
  double senseext_driver_delay_data;
   double compare_part_delay;
   double drive_mux_delay;
   double selb_delay;
   double data_output_delay;
   double drive_valid_delay;
   double precharge_delay;
   
} time_result_type;


double logtwo(double x);
double gatecap(double width,double wirelength);
double gatecappass(double width,double wirelength);
double draincap(double width,int nchannel,int stack);
double restowidth(double res,int nchannel);
double simple_array_power(int rows,int cols,int rports,int wports,int cache);
double simple_array_decoder_power(int rows,int cols,int rports,int wports,int cache);
double simple_array_bitline_power(int rows,int cols,int rports,int wports,int cache);
double simple_array_wordline_power(int rows,int cols,int rports,int wports,int cache);
double squarify_new(int rows,int cols);
/* register power stats */
void wattch_calculate_time(time_result_type*, time_parameter_type*);
void wattch_output_data(time_result_type*, time_parameter_type*);
void calculate_power(power_result_type*);
int pop_count(quad_t bits);
int pop_count_slow(quad_t bits);

bpower_result_type calculate_bpred_power() ;
#ifdef __cplusplus
}
#endif
#endif
