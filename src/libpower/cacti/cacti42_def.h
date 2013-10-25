#ifndef CACTI_DEF_H
#define CACTI_DEF_H
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

/*  The following are things you might want to change
 *  when compiling
 */

/*
 * The output can be in 'long' format, which shows everything, or
 * 'short' format, which is just what a program like 'grep' would
 * want to see
 */

#define LONG 1
#define SHORT 2

#define OUTPUTTYPE LONG

/*
 * Address bits in a word, and number of output bits from the cache
 */

/*
was: #define ADDRESS_BITS 32
now: I'm using 42 bits as in the Power4,
since that's bigger then the 36 bits on the Pentium 4
and 40 bits on the Opteron
*/
#define ADDRESS_BITS 42
/*
was: #define BITOUT 64
now: making it a commandline parameter
*/
//v4.2
//static int BITOUT;
int BITOUT;

/*dt: In addition to the tag bits, the tags also include 1 valid bit, 1 dirty bit, 2 bits for a 4-state
  cache coherency protocoll (MESI), 1 bit for MRU (change this to log(ways) for full LRU).
  So in total we have 1 + 1 + 2 + 1 = 5 */
#define EXTRA_TAG_BITS 5


/* limits on the various N parameters */

#define MAXDATAN 32          /* Maximum for Ndwl,Ndbl */
#define MAXTAGN 32          /* Maximum for Ndwl,Ndbl */
#define MAXSUBARRAYS 256    /* Maximum subarrays for data and tag arrays */
#define MAXDATASPD 32         /* Maximum for Nspd */
#define MAXTAGSPD 32         /* Maximum for Ntspd */



/*
 * The following scale factor can be used to scale between technologies.
 * To convert from 0.8um to 0.5um, make FUDGEFACTOR = 1.6
 */

double FUDGEFACTOR;
double FEATURESIZE;

#define PICOJ  (1e12)
#define PICOS  (1e-12)

/*===================================================================*/

/*
 * Cache layout parameters and process parameters
 */


/*
 * CMOS 0.8um model parameters
 *   - directly from Appendix II of tech report
 */

/*#define WORDWIRELENGTH (8+2*WIREPITCH*(EXTRAWRITEPORTS)+2*WIREPITCH*(EXTRAREADPORTS))*/
/*#define BITWIRELENGTH (16+2*WIREPITCH*(EXTRAWRITEPORTS+EXTRAREADPORTS))*/
/*
was:
#define WIRESPACING (2*FEATURESIZE)
#define WIREWIDTH (3*FEATURESIZE)
is: width and pitch are taken from the Intel IEDM 2004 paper on their 65nm process.
*/

#define WIRESPACING (1.6*FEATURESIZE)
#define WIREWIDTH (1.6*FEATURESIZE)
/*dt: I've taken the aspect ratio from the Intel paper on their 65nm process */
#define WIREHEIGTHRATIO	(1.8)

#define WIREPITCH (WIRESPACING+WIREWIDTH)
/*
#define Default_Cmetal 275e-18
*/
/*dt: The old Cmetal was calculated using SiO2 as dielectric (k = 3.9). Going by the Intel 65nm paper,
low-k dielectics are not at k=2.9. This is a very simple adjustment, as lots of other factors also go into the average
capacitance, but I don't have any better/more up to date data on wire distribution, etc. than what's been done for cacti 1.0.
So I'm doing a simple adjustment by 2.9/3.9 */
//#define Default_Cmetal (2.9/3.9*275e-18)
/*dt: changing this to reflect newer data */
/* 2.9: is the k value for the low-k dielectric used in 65nm Intel process
   3.9: is the k value of normal SiO2
   --> multiply by 2.9/3.9 to get new C values
   the Intel 130nm process paper mentioned 230fF/mm for M1 through M5 with k = 3.6
   So we get
   230*10^-15/mm * 10^-3 mm/1um * 3.9/3.6
*/
#define Default_Cmetal (2.9/3.9*230e-18*3.9/3.6)
#define Default_Rmetal 48e-3
/* dt: I'm assuming that even with all the process improvements,
copper will 'only' have 2/3 the sheet resistance of aluminum. */
#define Default_CopperSheetResistancePerMicroM 32e-3

/*dt: this number is calculated from the 2004 ITRS tables (RC delay and Wire sizes)*/
#define CRatiolocal_to_interm  (1.0/1.4)
/*dt: from ITRS 2004 using wire sizes, aspect ratios and effective resistivities for local and intermediate*/
#define RRatiolocal_to_interm  (1.0/2.04)

#define CRatiointerm_to_global  (1.0/1.9)
/*dt: from ITRS 2004 using wire sizes, aspect ratios and effective resistivities for local and intermediate*/
#define RRatiointerm_to_global  (1.0/3.05)

//v4.2
//static double WireRscaling;
//static double WireCscaling;

//static double Cwordmetal;
//static double Cbitmetal;
//static double Rwordmetal;
//static double Rbitmetal;

//static double TagCwordmetal;
//static double TagCbitmetal;
//static double TagRwordmetal;
//static double TagRbitmetal;

//static double GlobalCwordmetal;
//static double GlobalCbitmetal;
//static double GlobalRwordmetal;
//static double GlobalRbitmetal;
//static double FACwordmetal;
//static double FACbitmetal;
//static double FARwordmetal;
//static double FARbitmetal;

double WireRscaling;
double WireCscaling;

double Cwordmetal;
double Cbitmetal;
double Rwordmetal;
double Rbitmetal;

double TagCwordmetal;
double TagCbitmetal;
double TagRwordmetal;
double TagRbitmetal;

double GlobalCwordmetal;
double GlobalCbitmetal;
double GlobalRwordmetal;
double GlobalRbitmetal;
double FACwordmetal;
double FACbitmetal;
double FARwordmetal;
double FARbitmetal;


//v4.2
//static int muxover;
int muxover;

/* fF/um2 at 1.5V */
//#define Cndiffarea    0.137e-15

//v4.1: Making all constants static variables. Initially these variables are based
//off 0.8 micron process values; later on in init_tech_params function of leakage.c
//they are scaled to input tech node parameters

double Cndiffarea;

/* fF/um2 at 1.5V */
//#define Cpdiffarea    0.343e-15

double Cpdiffarea;

/* fF/um at 1.5V */
//#define Cndiffside    0.275e-15

double Cndiffside;

/* fF/um at 1.5V */
//#define Cpdiffside    0.275e-15
double Cpdiffside;

/* fF/um at 1.5V */
//#define Cndiffovlp    0.138e-15
double Cndiffovlp;

/* fF/um at 1.5V */
//#define Cpdiffovlp    0.138e-15
double Cpdiffovlp;

/* fF/um assuming 25% Miller effect */
//#define Cnoxideovlp   0.263e-15
double Cnoxideovlp;

/* fF/um assuming 25% Miller effect */
//#define Cpoxideovlp   0.338e-15
double Cpoxideovlp;

/* um */
//#define Leff          (0.8)
double Leff;

//#define inv_Leff	  1.25
double inv_Leff;

/* fF/um2 */
//#define Cgate         1.95e-15
double Cgate;

/* fF/um2 */
//#define Cgatepass     1.45e-15
double Cgatepass;

/* note that the value of Cgatepass will be different depending on
   whether or not the source and drain are at different potentials or
   the same potential.  The two values were averaged */

/* fF/um */
//#define Cpolywire	(0.25e-15)
double Cpolywire;

/* ohms*um of channel width */
//#define Rnchannelstatic	(25800)
double Rnchannelstatic;

/* ohms*um of channel width */
//#define Rpchannelstatic	(61200)
double Rpchannelstatic;

//#define Rnchannelon	(8751)
double Rnchannelon;

//#define Rpchannelon	(20160)
double Rpchannelon;


#define Vdd		5
//v4.2
//static double VddPow;
double VddPow;
/* Threshold voltages (as a proportion of Vdd)
   If you don't know them, set all values to 0.5 */

#define SizingRatio   0.33
#define VTHNAND       0.561
#define VTHFA1        0.452
#define VTHFA2        0.304
#define VTHFA3        0.420
#define VTHFA4        0.413
#define VTHFA5        0.405
#define VTHFA6        0.452
#define VSINV         0.452
#define VTHINV100x60  0.438   /* inverter with p=100,n=60 */
#define VTHINV360x240 0.420   /* inverter with p=360, n=240 */
#define VTHNAND60x90  0.561   /* nand with p=60 and three n=90 */
#define VTHNOR12x4x1  0.503   /* nor with p=12, n=4, 1 input */
#define VTHNOR12x4x2  0.452   /* nor with p=12, n=4, 2 inputs */
#define VTHNOR12x4x3  0.417   /* nor with p=12, n=4, 3 inputs */
#define VTHNOR12x4x4  0.390   /* nor with p=12, n=4, 4 inputs */
#define VTHOUTDRINV    0.437
#define VTHOUTDRNOR   0.379
#define VTHOUTDRNAND  0.63
#define VTHOUTDRIVE   0.425
#define VTHCOMPINV    0.437
#define VTHMUXNAND    0.548
#define VTHMUXDRV1    0.406
#define VTHMUXDRV2    0.334
#define VTHMUXDRV3    0.478
#define VTHEVALINV    0.452
#define VTHSENSEEXTDRV  0.438

#define VTHNAND60x120 0.522

/* transistor widths in um (as described in tech report, appendix 1) */
/* incorporating changes from eCACTI. Mostly device widths are now calculated with logical effort
and no longer static. Some changes to constants to reflect improving/changing circuit/device technology */

//#define Wdecdrivep	(360.0)
double Wdecdrivep;
//#define Wdecdriven	(240.0)
double Wdecdriven;
/*#define Wdec3to8n     120.0
#define Wdec3to8p     60.0
#define WdecNORn       2.4
#define WdecNORp      12.0
#define Wdecinvn      20.0
#define Wdecinvp      40.0 */


//#define Wworddrivemax 100.0
double Wworddrivemax;

double Waddrdrvn1;
double Waddrdrvp1;
double Waddrdrvn2;
double Waddrdrvp2;

double Wdecdrivep_second;
double Wdecdriven_second;
double Wdecdrivep_first;
double Wdecdriven_first;
double WdecdrivetreeN[10];
double Cdectreesegments[10], Rdectreesegments[10];
int    nr_dectreesegments;
double Wdec3to8n ;
double Wdec3to8p ;
double WdecNORn  ;
double WdecNORp  ;
double Wdecinvn  ;
double Wdecinvp  ;
double WwlDrvn ;
double WwlDrvp ;

double Wtdecdrivep_second;
double Wtdecdriven_second;
double Wtdecdrivep_first;
double Wtdecdriven_first;
double WtdecdrivetreeN[10];
double Ctdectreesegments[10], Rtdectreesegments[10];
int    nr_tdectreesegments;
double Wtdec3to8n ;
double Wtdec3to8p ;
double WtdecNORn  ;
double WtdecNORp  ;
double Wtdecinvn  ;
double Wtdecinvp  ;
double WtwlDrvn ;
double WtwlDrvp ;


/* #define Wmemcella	(2.4)
// added by MnM
// #define Wmemcellpmos (4.0)
// #define Wmemcellnmos (2.0)
*/

//#define Wmemcella	(0.9)
double Wmemcella;

/* added by MnM */
//#define Wmemcellpmos (0.65)
double Wmemcellpmos;
//#define Wmemcellnmos (2.0)
double Wmemcellnmos;


//#define Wmemcellbscale	2		/* means 2x bigger than Wmemcella */
int Wmemcellbscale;
/* #define Wbitpreequ	(80.0) */
double Wbitpreequ;
//#define Wpchmax		(25.0) /* precharge transistor sizes usually do not exceed 25 */
double Wpchmax;


/* #define Wbitmuxn	(10.0)
//#define WsenseQ1to4	(4.0) */

double Wpch;
double Wiso;
double WsenseEn;
double WsenseN;
double WsenseP;
double WsPch;
double WoBufN;
double WoBufP;

double WpchDrvp, WpchDrvn;
double WisoDrvp, WisoDrvn;
double WspchDrvp, WspchDrvn;
double WsenseEnDrvp, WsenseEnDrvn;

double WwrtMuxSelDrvn;
double WwrtMuxSelDrvp;
double WtwrtMuxSelDrvn;
double WtwrtMuxSelDrvp;

double Wtbitpreequ;
double Wtpch;
double Wtiso;
double WtsenseEn;
double WtsenseN;
double WtsenseP;
double WtoBufN;
double WtoBufP;
double WtsPch;

double WtpchDrvp, WtpchDrvn;
double WtisoDrvp, WtisoDrvn;
double WtspchDrvp, WtspchDrvn;
double WtsenseEnDrvp, WtsenseEnDrvn;

//#define Wcompinvp1	(10.0)
double Wcompinvp1;
//#define Wcompinvn1	(6.0)
double Wcompinvn1;
//#define Wcompinvp2	(20.0)
double Wcompinvp2;
//#define Wcompinvn2	(12.0)
double Wcompinvn2;
//#define Wcompinvp3	(40.0)
double Wcompinvp3;
//#define Wcompinvn3	(24.0)
double Wcompinvn3;
//#define Wevalinvp	(80.0)
double Wevalinvp;
//#define Wevalinvn	(40.0)
double Wevalinvn;

//#define Wfadriven    (50.0)
double Wfadriven;
//#define Wfadrivep    (100.0)
double Wfadrivep;
//#define Wfadrive2n    (200.0)
double Wfadrive2n;
//#define Wfadrive2p    (400.0)
double Wfadrive2p;
//#define Wfadecdrive1n    (5.0)
double Wfadecdrive1n;
//#define Wfadecdrive1p    (10.0)
double Wfadecdrive1p;
//#define Wfadecdrive2n    (20.0)
double Wfadecdrive2n;
//#define Wfadecdrive2p    (40.0)
double Wfadecdrive2p;
//#define Wfadecdriven    (50.0)
double Wfadecdriven;
//#define Wfadecdrivep    (100.0)
double Wfadecdrivep;
//#define Wfaprechn       (6.0)
double Wfaprechn;
//#define Wfaprechp       (10.0)
double Wfaprechp;
//#define Wdummyn         (10.0)
double Wdummyn;
//#define Wdummyinvn      (60.0)
double Wdummyinvn;
//#define Wdummyinvp      (80.0)
double Wdummyinvp;
//#define Wfainvn         (10.0)
double Wfainvn;
//#define Wfainvp         (20.0)
double Wfainvp;
//#define Waddrnandn      (50.0)
double Waddrnandn;
//#define Waddrnandp      (50.0)
double Waddrnandp;
//#define Wfanandn        (20.0)
double Wfanandn;
//#define Wfanandp        (30.0)
double Wfanandp;
//#define Wfanorn         (5.0)
double Wfanorn;
//#define Wfanorp         (10.0)
double Wfanorp;
//#define Wdecnandn       (10.0)
double Wdecnandn;
//#define Wdecnandp       (30.0)
double Wdecnandp;

//#define Wcompn		(10.0)
double Wcompn;
//#define Wcompp		(30.0)
double Wcompp;
//#define Wmuxdrv12n	(60.0)
double Wmuxdrv12n;
//#define Wmuxdrv12p	(100.0)
double Wmuxdrv12p;

/* #define WmuxdrvNANDn    (60.0)
//#define WmuxdrvNANDp    (80.0)
//#define WmuxdrvNORn	(40.0)
//#define WmuxdrvNORp	(100.0)
//#define Wmuxdrv3n	(80.0)
//#define Wmuxdrv3p	(200.0)
// #define Woutdrvseln	(24.0)
// #define Woutdrvselp	(40.0)
*/

double Coutdrvtreesegments[20], Routdrvtreesegments[20];
double WoutdrvtreeN[20];
int    nr_outdrvtreesegments;

double Cmuxdrvtreesegments[20], Rmuxdrvtreesegments[20];
double WmuxdrvtreeN[20];
int    nr_muxdrvtreesegments;

double WmuxdrvNANDn    ;
double WmuxdrvNANDp    ;
double WmuxdrvNORn	;
double WmuxdrvNORp	;
double Wmuxdrv3n	;
double Wmuxdrv3p	;
double Woutdrvseln	;
double Woutdrvselp	;


/* #define Woutdrvnandn	(10.0)
//#define Woutdrvnandp	(30.0)
//#define Woutdrvnorn	(5.0)
//#define Woutdrvnorp	(20.0)
//#define Woutdrivern	(48.0)
//#define Woutdriverp	(80.0)
*/

double Woutdrvnandn;
double Woutdrvnandp;
double Woutdrvnorn	;
double Woutdrvnorp	;
double Woutdrivern	;
double Woutdriverp	;


//#define Wsenseextdrv1p (80.0)
double Wsenseextdrv1p;
//#define Wsenseextdrv1n (40.0)
double Wsenseextdrv1n;
//#define Wsenseextdrv2p (240.0)
double Wsenseextdrv2p;
//#define Wsenseextdrv2n (160.0)
double Wsenseextdrv2n;


/* other stuff (from tech report, appendix 1) */

//#define krise		(0.4e-9)
double krise;
//#define tsensedata	(5.8e-10)
double tsensedata;
// #define psensedata      (0.025e-9)
//#define psensedata      (0.02e-9)
double psensedata;
//#define tsensescale     0.02e-10
double tsensescale;
//#define tsensetag	(2.6e-10)
double tsensetag;
// #define psensetag       (0.01e-9)
//#define psensetag	(0.016e-9)
double psensetag;
//#define tfalldata	(7e-10)
double tfalldata;
//#define tfalltag	(7e-10)
double tfalltag;
#define Vbitpre		(3.3)
//v4.2
//static double VbitprePow;
double VbitprePow;
#define Vt		(1.09)
/*
was: #define Vbitsense	(0.10)
now: 50mV seems to be the norm as of 2005
*/
#define Vbitsense	(0.05*Vdd)
#define Vbitswing	(0.20*Vdd)

/* bit width of RAM cell in um */
/*
was:
#define BitWidth	(8.0)
*/
//#define BitWidth	7.746*0.8
double BitWidth;

/* bit height of RAM cell in um */
/*
was:
#define BitHeight	(16.0)
*/
//#define BitHeight	2*7.746*0.8
double BitHeight;

//#define Cout		(0.5e-12)
double Cout;

int dualVt ;
int explore;
/*===================================================================*/

/*
 * The following are things you probably wouldn't want to change.
 */


#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL 0
#endif
#define OK 1
#define ERROR 0
#define BIGNUM 1e30
#define DIVIDE(a,b) ((b)==0)? 0:(a)/(b)
#define MAX(a,b) (((a)>(b))?(a):(b))

#define WAVE_PIPE 3
#define MAX_COL_MUX 16

/* Used to communicate with the horowitz model */

#define RISE 1
#define FALL 0
#define NCH  1
#define PCH  0


/* Used to pass values around the program */

/*dt: maximum numbers of entries in the
      caching structures of the tag calculations
*/
#define MAX_CACHE_ENTRIES 512

//v4.2
//static int sequential_access_flag;
//static int fast_cache_access_flag;
int sequential_access_flag;
int fast_cache_access_flag;
int pure_sram_flag; //Changed from static int to just int as value wasn't getting passed through to
//area function in area.c

#define EPSILON 0.5 //v4.1: This constant is being used in order to fix floating point -> integer
//conversion problems that were occuring within CACTI. Typical problem that was occuring was
//that with different compilers a floating point number like 3.0 would get represented as either
//2.9999....or 3.00000001 and then the integer part of the floating point number (3.0) would
//be computed differently depending on the compiler. What we are doing now is to replace
//int (x) with (int) (x+EPSILON) where EPSILON is 0.5. This would fix such problems. Note that
//this works only when x is an integer >= 0.

#endif
