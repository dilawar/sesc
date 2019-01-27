/* I inclued this copyright since we're using Cacti for some stuff */

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

#include <math.h>
#include "machine.h"
#include "wattch_power.h"
#include "wattch_cache.h"
#include <assert.h>

#define SensePowerfactor (Mhz)*(Vdd/2)*(Vdd/2)
#define Sense2Powerfactor (Mhz)*(2*.3+.1*Vdd)
#define Powerfactor (Mhz)*Vdd*Vdd
#define LowSwingPowerfactor (Mhz)*.2*.2
/* set scale for crossover (vdd->gnd) currents */
double crossover_scaling = 1.2;
/* set non-ideal turnoff percentage */
double turnoff_factor = 0.1;

#define MSCALE (LSCALE * .624 / .2250)

/*----------------------------------------------------------------------*/

/* static power model results */
power_result_type power;

int pow2(int x) {
  return((int)pow(2.0,(double)x));
}

double logfour(x)
     double x;
{
  if (x<=0) fprintf(stderr,"%e\n",x);
  return( (double) (log(x)/log(4.0)) );
}

#ifdef HOST_HAS_QUAD2
/* safer pop count to validate the fast algorithm */
int pop_count_slow(quad_t bits)
{
  int count = 0; 
  quad_t tmpbits = bits; 
  while (tmpbits) { 
    if (tmpbits & 1) ++count; 
    tmpbits >>= 1; 
  } 
  return count; 
}

/* fast pop count */
int pop_count(quad_t bits)
{
#define T unsigned long long
#define ONES ((T)(-1)) 
#define TWO(k) ((T)1 << (k)) 
#define CYCL(k) (ONES/(1 + (TWO(TWO(k))))) 
#define BSUM(x,k) ((x)+=(x) >> TWO(k), (x) &= CYCL(k)) 
  quad_t x = bits; 
  x = (x & CYCL(0)) + ((x>>TWO(0)) & CYCL(0)); 
  x = (x & CYCL(1)) + ((x>>TWO(1)) & CYCL(1)); 
  BSUM(x,2); 
  BSUM(x,3); 
  BSUM(x,4); 
  BSUM(x,5); 
  return x; 
}
#endif

int opcode_length = 8;
int inst_length = 32;

int nvreg_width;
int npreg_width;

double global_clockcap;

/* compute bitline activity factors which we use to scale bitline power 
   Here it is very important whether we assume 0's or 1's are
   responsible for dissipating power in pre-charged stuctures. (since
   most of the bits are 0's, we assume the design is power-efficient
   enough to allow 0's to _not_ discharge 
*/
double compute_af(counter_t num_pop_count_cycle,counter_t total_pop_count_cycle,int pop_width) {
  double avg_pop_count;
  double af,af_b;

  if(num_pop_count_cycle)
    avg_pop_count = (double)total_pop_count_cycle / (double)num_pop_count_cycle;
  else
    avg_pop_count = 0;

  af = avg_pop_count / (double)pop_width;
  
  af_b = 1.0 - af;

  /*  printf("af == %f%%, af_b == %f%%, total_pop == %d, num_pop == %d\n",100*af,100*af_b,total_pop_count_cycle,num_pop_count_cycle); */

  return(af_b);
}

/* compute power statistics on each cycle, for each conditional clocking style.  Obviously
most of the speed penalty comes here, so if you don't want per-cycle power estimates
you could post-process 

See README.wattch for details on the various clock gating styles.

*/


/* this routine takes the number of rows and cols of an array structure
   and attemps to make it make it more of a reasonable circuit structure
   by trying to make the number of rows and cols as close as possible.
   (scaling both by factors of 2 in opposite directions).  it returns
   a scale factor which is the amount that the rows should be divided
   by and the columns should be multiplied by.
*/
int squarify(int rows, int cols)
{
  int scale_factor = 1;

  if(rows == cols)
    return 1;

  /*
  printf("init rows == %d\n",rows);
  printf("init cols == %d\n",cols);
  */

  while(rows > cols) {
    rows = rows/2;
    cols = cols*2;

    /*
    printf("rows == %d\n",rows);
    printf("cols == %d\n",cols);
    printf("scale_factor == %d (2^ == %d)\n\n",scale_factor,(int)pow(2.0,(double)scale_factor));
    */

    if (rows/2 <= cols)
      return((int)pow(2.0,(double)scale_factor));
    scale_factor++;
  }

  return 1;
}

/* could improve squarify to work when rows < cols */

double squarify_new(int rows, int cols)
{
  double scale_factor = 0.0;

  if(rows==cols)
    return(pow(2.0,scale_factor));

  while(rows > cols) {
    rows = rows/2;
    cols = cols*2;
    if (rows <= cols)
      return(pow(2.0,scale_factor));
    scale_factor++;
  }

  while(cols > rows) {
    rows = rows*2;
    cols = cols/2;
    if (cols <= rows)
      return(pow(2.0,scale_factor));
    scale_factor--;
  }

  return 1;

}

void dump_power_stats(power_result_type *power)
{
  double total_power;
  double bpred_power;
  double rename_power;
  double rat_power;
  double dcl_power;
  double ldq_power;
  double stq_power;
  double window_power;
  double wakeup_power;
  double rs_power;
  double ldq_wakeup_power;
  double ldq_rs_power;
  double stq_wakeup_power;
  double stq_rs_power;
  double regfile_power;
  double reorder_power;
  double icache_power;
  double dcache_power;
  double dcache2_power;
  double dtlb_power;
  double itlb_power;
  double ambient_power = 2.0;

  icache_power = power->icache_power;

  dcache_power = power->dcache_power;

  dcache2_power = power->dcache2_power;

  itlb_power = power->itlb;
  dtlb_power = power->dtlb;

  bpred_power = power->btb + power->local_predict + power->global_predict + 
    power->chooser + power->ras;

  rat_power = power->rat_decoder + 
    power->rat_wordline + power->rat_bitline + power->rat_senseamp;

  dcl_power = power->dcl_compare + power->dcl_pencode;

  rename_power = power->rat_power + power->dcl_power + power->inst_decoder_power;

  wakeup_power = power->wakeup_tagdrive + power->wakeup_tagmatch + 
    power->wakeup_ormatch;
   
  rs_power = power->rs_decoder + 
    power->rs_wordline + power->rs_bitline + power->rs_senseamp;

  window_power = wakeup_power + rs_power + power->selection;

  ldq_rs_power = power->ldq_rs_decoder + 
    power->ldq_rs_wordline + power->ldq_rs_bitline + power->ldq_rs_senseamp;

  stq_rs_power = power->stq_rs_decoder + 
    power->stq_rs_wordline + power->stq_rs_bitline + power->stq_rs_senseamp;

  ldq_wakeup_power = power->ldq_wakeup_tagdrive + 
    power->ldq_wakeup_tagmatch + power->ldq_wakeup_ormatch;

  stq_wakeup_power = power->stq_wakeup_tagdrive + 
    power->stq_wakeup_tagmatch + power->stq_wakeup_ormatch;

  ldq_power = ldq_wakeup_power + ldq_rs_power;

  stq_power = stq_wakeup_power + stq_rs_power;

  reorder_power = power->reorder_decoder + 
    power->reorder_wordline + power->reorder_bitline + 
    power->reorder_senseamp;

  regfile_power = power->regfile_decoder + 
    power->regfile_wordline + power->regfile_bitline + 
    power->regfile_senseamp;

  total_power = bpred_power + rename_power + window_power + regfile_power +
    power->resultbus + ldq_power + stq_power +
    icache_power + dcache_power + dcache2_power + 
    dtlb_power + itlb_power + power->clock_power + power->ialu_power +
    power->falu_power;

  fprintf(stderr,"\nProcessor Parameters:\n");
  fprintf(stderr,"Issue Width: %d\n",ruu_issue_width);
  fprintf(stderr,"Window Size: %d\n",RUU_size);
  fprintf(stderr,"Number of Virtual Registers: %d\n",MD_NUM_IREGS);
  fprintf(stderr,"Number of Physical Registers: %d\n",RUU_size);
  fprintf(stderr,"Datapath Width: %d\n",data_width);

  fprintf(stderr,"Total Power Consumption: %g\n",total_power+ambient_power);
  fprintf(stderr,"Branch Predictor Power Consumption: %g  (%.3g%%)\n",bpred_power,100*bpred_power/total_power);
  fprintf(stderr," branch target buffer power (W): %g\n",power->btb);
  fprintf(stderr," local predict power (W): %g\n",power->local_predict);
  fprintf(stderr," global predict power (W): %g\n",power->global_predict);
  fprintf(stderr," chooser power (W): %g\n",power->chooser);
  fprintf(stderr," RAS power (W): %g\n",power->ras);
  fprintf(stderr,"Rename Logic Power Consumption: %g  (%.3g%%)\n",rename_power,100*rename_power/total_power);
  fprintf(stderr," Instruction Decode Power (W): %g\n",power->inst_decoder_power);
  fprintf(stderr," RAT decode_power (W): %g\n",power->rat_decoder);
  fprintf(stderr," RAT wordline_power (W): %g\n",power->rat_wordline);
  fprintf(stderr," RAT bitline_power (W): %g\n",power->rat_bitline);
  fprintf(stderr," DCL Comparators (W): %g\n",power->dcl_compare);
  fprintf(stderr,"Instruction Window Power Consumption: %g  (%.3g%%)\n",window_power,100*window_power/total_power);
  fprintf(stderr," tagdrive (W): %g\n",power->wakeup_tagdrive);
  fprintf(stderr," tagmatch (W): %g\n",power->wakeup_tagmatch);
  fprintf(stderr," Selection Logic (W): %g\n",power->selection);
  fprintf(stderr," decode_power (W): %g\n",power->rs_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->rs_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->rs_bitline);
  fprintf(stderr,"Load Queue Power Consumption: %g  (%.3g%%)\n",ldq_power,100*ldq_power/total_power);
  fprintf(stderr," wakeup (W): %g\n",power->ldq_wakeup_power);
  fprintf(stderr," RS (W): %g\n",power->ldq_rs_power);
  fprintf(stderr," tagdrive (W): %g\n",power->ldq_wakeup_tagdrive);
  fprintf(stderr," tagmatch (W): %g\n",power->ldq_wakeup_tagmatch);
  fprintf(stderr," decode_power (W): %g\n",power->ldq_rs_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->ldq_rs_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->ldq_rs_bitline);
  fprintf(stderr,"Store Queue Power Consumption: %g  (%.3g%%)\n",stq_power,100*stq_power/total_power);
  fprintf(stderr," wakeup (W): %g\n",power->stq_wakeup_power);
  fprintf(stderr," RS (W): %g\n",power->stq_rs_power);
  fprintf(stderr," tagdrive (W): %g\n",power->stq_wakeup_tagdrive);
  fprintf(stderr," tagmatch (W): %g\n",power->stq_wakeup_tagmatch);
  fprintf(stderr," decode_power (W): %g\n",power->stq_rs_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->stq_rs_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->stq_rs_bitline);

  fprintf(stderr,"Arch. Register File Power Consumption: %g  (%.3g%%)\n",regfile_power,100*regfile_power/total_power);
  fprintf(stderr," decode_power (W): %g\n",power->regfile_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->regfile_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->regfile_bitline);
  fprintf(stderr,"Result Bus Power Consumption: %g  (%.3g%%)\n",power->resultbus,100*power->resultbus/total_power);
  fprintf(stderr,"Total Clock Power: %g  (%.3g%%)\n",power->clock_power,100*power->clock_power/total_power);
  fprintf(stderr,"Int ALU Power: %g  (%.3g%%)\n",power->ialu_power,100*power->ialu_power/total_power);
  fprintf(stderr,"FP ALU Power: %g  (%.3g%%)\n",power->falu_power,100*power->falu_power/total_power);
  fprintf(stderr,"Instruction Cache Power Consumption: %g  (%.3g%%)\n",icache_power,100*icache_power/total_power);
  fprintf(stderr," decode_power (W): %g\n",power->icache_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->icache_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->icache_bitline);
  fprintf(stderr," senseamp_power (W): %g\n",power->icache_senseamp);
  fprintf(stderr," tagarray_power (W): %g\n",power->icache_tagarray);
  fprintf(stderr,"Itlb_power (W): %g (%.3g%%)\n",power->itlb,100*power->itlb/total_power);
  fprintf(stderr,"Data Cache Power Consumption: %g  (%.3g%%)\n",dcache_power,100*dcache_power/total_power);
  fprintf(stderr," decode_power (W): %g\n",power->dcache_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->dcache_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->dcache_bitline);
  fprintf(stderr," senseamp_power (W): %g\n",power->dcache_senseamp);
  fprintf(stderr," tagarray_power (W): %g\n",power->dcache_tagarray);
  fprintf(stderr,"Dtlb_power (W): %g (%.3g%%)\n",power->dtlb,100*power->dtlb/total_power);
  fprintf(stderr,"Level 2 Cache Power Consumption: %g (%.3g%%)\n",dcache2_power,100*dcache2_power/total_power);
  fprintf(stderr," decode_power (W): %g\n",power->dcache2_decoder);
  fprintf(stderr," wordline_power (W): %g\n",power->dcache2_wordline);
  fprintf(stderr," bitline_power (W): %g\n",power->dcache2_bitline);
  fprintf(stderr," senseamp_power (W): %g\n",power->dcache2_senseamp);
  fprintf(stderr," tagarray_power (W): %g\n",power->dcache2_tagarray);
}

/*======================================================================*/



/* 
 * This part of the code contains routines for each section as
 * described in the tech report.  See the tech report for more details
 * and explanations */

/*----------------------------------------------------------------------*/

double driver_size(double driving_cap, double desiredrisetime) {
  double nsize, psize;
  double Rpdrive; 

  Rpdrive = desiredrisetime/(driving_cap*log(VSINV)*-1.0);
  psize = restowidth(Rpdrive,PCH);
  nsize = restowidth(Rpdrive,NCH);
  if (psize > Wworddrivemax) {
    psize = Wworddrivemax;
  }
  if (psize < 4.0 * LSCALE)
    psize = 4.0 * LSCALE;

  return (psize);

}

/* Decoder delay:  (see section 6.1 of tech report) */

double array_decoder_power(int rows,int cols,double predeclength,int rports,int wports,int cache)
{
  double Ctotal=0;
  double Ceq=0;
  int numstack;
  int decode_bits=0;
  int ports;
  double rowsb;

  /* read and write ports are the same here */
  ports = rports + wports;

  rowsb = (double)rows;

  /* number of input bits to be decoded */
  decode_bits=ceil((logtwo(rowsb)));

  /* First stage: driving the decoders */

  /* This is the capacitance for driving one bit (and its complement).
     -There are #rowsb 3->8 decoders contributing gatecap.
     - 2.0 factor from 2 identical sets of drivers in parallel
  */
  Ceq = 2.0*(draincap(Wdecdrivep,PCH,1)+draincap(Wdecdriven,NCH,1)) +
    gatecap(Wdec3to8n+Wdec3to8p,10.0)*rowsb;

  /* There are ports * #decode_bits total */
  Ctotal+=ports*decode_bits*Ceq;

  if(verbose)
    fprintf(stderr,"Decoder -- Driving decoders            == %g\n",.3*Ctotal*Powerfactor);

  /* second stage: driving a bunch of nor gates with a nand 
     numstack is the size of the nor gates -- ie. a 7-128 decoder has
     3-input NAND followed by 3-input NOR  */

  numstack = ceil((1.0/3.0)*logtwo(rows));

  if (numstack<=0) numstack = 1;
  if (numstack>5) numstack = 5;

  /* There are #rowsb NOR gates being driven*/
  Ceq = (3.0*draincap(Wdec3to8p,PCH,1) +draincap(Wdec3to8n,NCH,3) +
	 gatecap(WdecNORn+WdecNORp,((numstack*40)+20.0)))*rowsb;

  Ctotal+=ports*Ceq;

  if(verbose)
    fprintf(stderr,"Decoder -- Driving nor w/ nand         == %g\n",.3*ports*Ceq*Powerfactor);

  /* Final stage: driving an inverter with the nor 
     (inverter preceding wordline driver) -- wordline driver is in the next section*/

  Ceq = (gatecap(Wdecinvn+Wdecinvp,20.0)+
	 numstack*draincap(WdecNORn,NCH,1)+
         draincap(WdecNORp,PCH,numstack));

  if(verbose)
    fprintf(stderr,"Decoder -- Driving inverter w/ nor     == %g\n",.3*ports*Ceq*Powerfactor);

  Ctotal+=ports*Ceq;

  /* assume Activity Factor == .3  */

  return(.3*Ctotal*Powerfactor);
}

double simple_array_decoder_power(int rows,int cols,int rports,int wports,int cache)
{
  double predeclength=0.0;
  return(array_decoder_power(rows,cols,predeclength,rports,wports,cache));
}


double array_wordline_power(int rows,int cols,double wordlinelength,int rports,int wports,int cache)
{
  double Ctotal=0;
  double Ceq=0;
  double Cline=0;
  double Cliner, Clinew=0;
  double desiredrisetime,psize,nsize;
  int ports;
  double colsb;

  ports = rports+wports;

  colsb = (double)cols;

  /* Calculate size of wordline drivers assuming rise time == Period / 8 
     - estimate cap on line 
     - compute min resistance to achieve this with RC 
     - compute width needed to achieve this resistance */

  desiredrisetime = Period/16;
  Cline = (gatecappass(Wmemcellr,1.0))*colsb + wordlinelength*CM3metal;
  psize = driver_size(Cline,desiredrisetime);
  
  /* how do we want to do p-n ratioing? -- here we just assume the same ratio 
     from an inverter pair  */
  nsize = psize * Wdecinvn/Wdecinvp; 
  
  if(verbose)
    fprintf(stderr,"Wordline Driver Sizes -- nsize == %f, psize == %f\n",nsize,psize);

  Ceq = draincap(Wdecinvn,NCH,1) + draincap(Wdecinvp,PCH,1) +
    gatecap(nsize+psize,20.0);

  Ctotal+=ports*Ceq;

  if(verbose)
    fprintf(stderr,"Wordline -- Inverter -> Driver         == %g\n",ports*Ceq*Powerfactor);

  /* Compute caps of read wordline and write wordlines 
     - wordline driver caps, given computed width from above
     - read wordlines have 1 nmos access tx, size ~4
     - write wordlines have 2 nmos access tx, size ~2
     - metal line cap
  */

  Cliner = (gatecappass(Wmemcellr,(BitWidth-2*Wmemcellr)/2.0))*colsb+
    wordlinelength*CM3metal+
    2.0*(draincap(nsize,NCH,1) + draincap(psize,PCH,1));
  Clinew = (2.0*gatecappass(Wmemcellw,(BitWidth-2*Wmemcellw)/2.0))*colsb+
    wordlinelength*CM3metal+
    2.0*(draincap(nsize,NCH,1) + draincap(psize,PCH,1));

  if(verbose) {
    fprintf(stderr,"Wordline -- Line                       == %g\n",1e12*Cline);
    fprintf(stderr,"Wordline -- Line -- access -- gatecap  == %g\n",1e12*colsb*2*gatecappass(Wmemcella,(BitWidth-2*Wmemcella)/2.0));
    fprintf(stderr,"Wordline -- Line -- driver -- draincap == %g\n",1e12*draincap(nsize,NCH,1) + draincap(psize,PCH,1));
    fprintf(stderr,"Wordline -- Line -- metal              == %g\n",1e12*wordlinelength*CM3metal);
  }
  Ctotal+=rports*Cliner+wports*Clinew;

  /* AF == 1 assuming a different wordline is charged each cycle, but only
     1 wordline (per port) is actually used */

  return(Ctotal*Powerfactor);
}

double simple_array_wordline_power(rows,cols,rports,wports,cache)
     int rows,cols;
     int rports,wports;
     int cache;
{
  double wordlinelength;
  int ports = rports + wports;
  wordlinelength = cols *  (RegCellWidth + 2 * ports * BitlineSpacing);
  return(array_wordline_power(rows,cols,wordlinelength,rports,wports,cache));
}


double array_bitline_power(rows,cols,bitlinelength,rports,wports,cache)
     int rows,cols;
     double bitlinelength;
     int rports,wports;
     int cache;
{
  double Ctotal=0;
  double Ccolmux=0;
  double Cbitrowr=0;
  double Cbitroww=0;
  double Cprerow=0;
  double Cwritebitdrive=0;
  double Cpregate=0;
  double Cliner=0;
  double Clinew=0;
  int ports;
  double rowsb;
  double colsb;

  double desiredrisetime, Cline, psize, nsize;

  ports = rports + wports;

  rowsb = (double)rows;
  colsb = (double)cols;

  /* Draincaps of access tx's */

  Cbitrowr = draincap(Wmemcellr,NCH,1);
  Cbitroww = draincap(Wmemcellw,NCH,1);

  /* Cprerow -- precharge cap on the bitline
     -simple scheme to estimate size of pre-charge tx's in a similar fashion
      to wordline driver size estimation.
     -FIXME: it would be better to use precharge/keeper pairs, i've omitted this
      from this version because it couldn't autosize as easily.
  */

  desiredrisetime = Period/8;

  Cline = rowsb*Cbitrowr+CM2metal*bitlinelength;
  psize = driver_size(Cline,desiredrisetime);

  /* compensate for not having an nmos pre-charging */
  psize = psize + psize * Wdecinvn/Wdecinvp; 

  if(verbose)
    printf("Cprerow auto   == %g (psize == %g)\n",draincap(psize,PCH,1),psize);

  Cprerow = draincap(psize,PCH,1);

  /* Cpregate -- cap due to gatecap of precharge transistors -- tack this
     onto bitline cap, again this could have a keeper */
  Cpregate = 4.0*gatecap(psize,10.0);
  global_clockcap+=rports*cols*2.0*Cpregate;

  /* Cwritebitdrive -- write bitline drivers are used instead of the precharge
     stuff for write bitlines
     - 2 inverter drivers within each driver pair */

  Cline = rowsb*Cbitroww+CM2metal*bitlinelength;

  psize = driver_size(Cline,desiredrisetime);
  nsize = psize * Wdecinvn/Wdecinvp; 

  Cwritebitdrive = 2.0*(draincap(psize,PCH,1)+draincap(nsize,NCH,1));

  /* 
     reg files (cache==0) 
     => single ended bitlines (1 bitline/col)
     => AFs from pop_count
     caches (cache ==1)
     => double-ended bitlines (2 bitlines/col)
     => AFs = .5 (since one of the two bitlines is always charging/discharging)
  */

#ifdef STATIC_AF
  if (cache == 0) {
    /* compute the total line cap for read/write bitlines */
    Cliner = rowsb*Cbitrowr+CM2metal*bitlinelength+Cprerow;
    Clinew = rowsb*Cbitroww+CM2metal*bitlinelength+Cwritebitdrive;

    /* Bitline inverters at the end of the bitlines (replaced w/ sense amps
       in cache styles) */
    Ccolmux = gatecap(MSCALE*(29.9+7.8),0.0)+gatecap(MSCALE*(47.0+12.0),0.0);
    Ctotal+=(1.0-POPCOUNT_AF)*rports*cols*(Cliner+Ccolmux+2.0*Cpregate);
    Ctotal+=.3*wports*cols*(Clinew+Cwritebitdrive);
  } 
  else { 
    Cliner = rowsb*Cbitrowr+CM2metal*bitlinelength+Cprerow + draincap(Wbitmuxn,NCH,1);
    Clinew = rowsb*Cbitroww+CM2metal*bitlinelength+Cwritebitdrive;
    Ccolmux = (draincap(Wbitmuxn,NCH,1))+2.0*gatecap(WsenseQ1to4,10.0);
    Ctotal+=.5*rports*2.0*cols*(Cliner+Ccolmux+2.0*Cpregate);
    Ctotal+=.5*wports*2.0*cols*(Clinew+Cwritebitdrive);
  }
#else
  if (cache == 0) {
    /* compute the total line cap for read/write bitlines */
    Cliner = rowsb*Cbitrowr+CM2metal*bitlinelength+Cprerow;
    Clinew = rowsb*Cbitroww+CM2metal*bitlinelength+Cwritebitdrive;

    /* Bitline inverters at the end of the bitlines (replaced w/ sense amps
       in cache styles) */
    Ccolmux = gatecap(MSCALE*(29.9+7.8),0.0)+gatecap(MSCALE*(47.0+12.0),0.0);
    Ctotal += rports*cols*(Cliner+Ccolmux+2.0*Cpregate);
    Ctotal += .3*wports*cols*(Clinew+Cwritebitdrive);
  } 
  else { 
    Cliner = rowsb*Cbitrowr+CM2metal*bitlinelength+Cprerow + draincap(Wbitmuxn,NCH,1);
    Clinew = rowsb*Cbitroww+CM2metal*bitlinelength+Cwritebitdrive;
    Ccolmux = (draincap(Wbitmuxn,NCH,1))+2.0*gatecap(WsenseQ1to4,10.0);
    Ctotal+=.5*rports*2.0*cols*(Cliner+Ccolmux+2.0*Cpregate);
    Ctotal+=.5*wports*2.0*cols*(Clinew+Cwritebitdrive);
  }
#endif

  if(verbose) {
    fprintf(stderr,"Bitline -- Precharge                   == %g\n",1e12*Cpregate);
    fprintf(stderr,"Bitline -- Line                        == %g\n",1e12*(Cliner+Clinew));
    fprintf(stderr,"Bitline -- Line -- access draincap     == %g\n",1e12*rowsb*Cbitrowr);
    fprintf(stderr,"Bitline -- Line -- precharge draincap  == %g\n",1e12*Cprerow);
    fprintf(stderr,"Bitline -- Line -- metal               == %g\n",1e12*bitlinelength*CM2metal);
    fprintf(stderr,"Bitline -- Colmux                      == %g\n",1e12*Ccolmux);

    fprintf(stderr,"\n");
  }


  if(cache==0)
    return(Ctotal*Powerfactor);
  else
    return(Ctotal*SensePowerfactor*.4);
  
}


double simple_array_bitline_power(rows,cols,rports,wports,cache)
     int rows,cols;
     int rports,wports;
     int cache;
{
  double bitlinelength;

  int ports = rports + wports;

  bitlinelength = rows * (RegCellHeight + ports * WordlineSpacing);

  return (array_bitline_power(rows,cols,bitlinelength,rports,wports,cache));

}

/* estimate senseamp power dissipation in cache structures (Zyuban's method) */
double senseamp_power(int cols)
{
  return((double)cols * Vdd/8 * .5e-3);
}

/* estimate comparator power consumption (this comparator is similar
   to the tag-match structure in a CAM */
double compare_cap(int compare_bits)
{
  double c1, c2;
  /* bottom part of comparator */
  c2 = (compare_bits)*(draincap(Wcompn,NCH,1)+draincap(Wcompn,NCH,2))+
    draincap(Wevalinvp,PCH,1) + draincap(Wevalinvn,NCH,1);

  /* top part of comparator */
  c1 = (compare_bits)*(draincap(Wcompn,NCH,1)+draincap(Wcompn,NCH,2)+
		       draincap(Wcomppreequ,NCH,1)) +
    gatecap(WdecNORn,1.0)+
    gatecap(WdecNORp,3.0);

  return(c1 + c2);
}

/* power of depency check logic */
double dcl_compare_power(int compare_bits)
{
  double Ctotal;
  int num_comparators;
  
  num_comparators = (ruu_decode_width - 1) * (ruu_decode_width);

  Ctotal = num_comparators * compare_cap(compare_bits);

  return(Ctotal*Powerfactor*AF);
}

double simple_array_power(rows,cols,rports,wports,cache)
     int rows,cols;
     int rports,wports;
     int cache;
{
  if(cache==0)
    return( simple_array_decoder_power(rows,cols,rports,wports,cache)+
	    simple_array_wordline_power(rows,cols,rports,wports,cache)+
	    simple_array_bitline_power(rows,cols,rports,wports,cache));
  else
    return( simple_array_decoder_power(rows,cols,rports,wports,cache)+
	    simple_array_wordline_power(rows,cols,rports,wports,cache)+
	    simple_array_bitline_power(rows,cols,rports,wports,cache)+
	    senseamp_power(cols));
}


double cam_tagdrive(rows,cols,rports,wports)
     int rows,cols,rports,wports;
{
  double Ctotal, Ctlcap, Cblcap, Cwlcap;
  double taglinelength;
  double wordlinelength;
  double nsize, psize;
  int ports;
  Ctotal=0;

  ports = rports + wports;

  taglinelength = rows * 
    (CamCellHeight + ports * MatchlineSpacing);

  wordlinelength = cols * 
    (CamCellWidth + ports * TaglineSpacing);

  /* Compute tagline cap */
  Ctlcap = Cmetal * taglinelength + 
    rows * gatecappass(Wcomparen2,2.0) +
    draincap(Wcompdrivern,NCH,1)+draincap(Wcompdriverp,PCH,1);

  /* Compute bitline cap (for writing new tags) */
  Cblcap = Cmetal * taglinelength +
    rows * draincap(Wmemcellr,NCH,2);

  /* autosize wordline driver */
  psize = driver_size(Cmetal * wordlinelength + 2 * cols * gatecap(Wmemcellr,2.0),Period/8);
  nsize = psize * Wdecinvn/Wdecinvp; 

  /* Compute wordline cap (for writing new tags) */
  Cwlcap = Cmetal * wordlinelength + 
    draincap(nsize,NCH,1)+draincap(psize,PCH,1) +
    2 * cols * gatecap(Wmemcellr,2.0);
    
  Ctotal += (rports * cols * 2 * Ctlcap) + 
    (wports * ((cols * 2 * Cblcap) + (rows * Cwlcap)));

  return(Ctotal*Powerfactor*AF);
}

double cam_tagmatch(rows,cols,rports,wports)
     int rows,cols,rports,wports;
{
  double Ctotal, Cmlcap;
  double matchlinelength;
  int ports;
  Ctotal=0;

  ports = rports + wports;

  matchlinelength = cols * 
    (CamCellWidth + ports * TaglineSpacing);

  Cmlcap = 2 * cols * draincap(Wcomparen1,NCH,2) + 
    Cmetal * matchlinelength + draincap(Wmatchpchg,NCH,1) +
    gatecap(Wmatchinvn+Wmatchinvp,10.0) +
    gatecap(Wmatchnandn+Wmatchnandp,10.0);

  Ctotal += rports * rows * Cmlcap;

  global_clockcap += rports * rows * gatecap(Wmatchpchg,5.0);
  
  /* noring the nanded match lines */
  if(ruu_issue_width >= 8)
    Ctotal += 2 * gatecap(Wmatchnorn+Wmatchnorp,10.0);

  return(Ctotal*Powerfactor*AF);
}

double cam_array(rows,cols,rports,wports)
     int rows,cols,rports,wports;
{
  return(cam_tagdrive(rows,cols,rports,wports) +
	 cam_tagmatch(rows,cols,rports,wports));
}


double selection_power(int win_entries)
{
  double Ctotal, Cor, Cpencode;
  int num_arbiter=1;

  Ctotal=0;

  while(win_entries > 4)
    {
      win_entries = (int)ceil((double)win_entries / 4.0);
      num_arbiter += win_entries;
    }

  Cor = 4 * draincap(WSelORn,NCH,1) + draincap(WSelORprequ,PCH,1);

  Cpencode = draincap(WSelPn,NCH,1) + draincap(WSelPp,PCH,1) + 
    2*draincap(WSelPn,NCH,1) + draincap(WSelPp,PCH,2) + 
    3*draincap(WSelPn,NCH,1) + draincap(WSelPp,PCH,3) + 
    4*draincap(WSelPn,NCH,1) + draincap(WSelPp,PCH,4) + 
    4*gatecap(WSelEnn+WSelEnp,20.0) + 
    4*draincap(WSelEnn,NCH,1) + 4*draincap(WSelEnp,PCH,1);

  Ctotal += ruu_issue_width * num_arbiter*(Cor+Cpencode);

  return(Ctotal*Powerfactor*AF);
}

/* very rough clock power estimates */
double total_clockpower(double die_length)
{
  int i;
  double clocklinelength;
  double Cline,Cline2,Ctotal;
  double pipereg_clockcap=0;
  double global_buffercap = 0;
  double Clockpower;

  double num_piperegs =0;

  int npreg_width = (int)ceil(logtwo((double)RUU_size));

  for(i =0; i<decode_stages ; i++)
    num_piperegs += ruu_fetch_width*inst_length + data_width;

  for(i =0; i<rename_stages ; i++)
    num_piperegs += ruu_decode_width*(inst_length + 3 * MD_NUM_IREGS);

  for(i =0; i<wakeup_stages ; i++)
    num_piperegs += ruu_issue_width*(3 * npreg_width + pow2(opcode_length));

  /* Additional stages */
  num_piperegs += ruu_issue_width*(2*data_width + pow2(opcode_length));
  num_piperegs += ruu_issue_width*(data_width + pow2(opcode_length));
  num_piperegs += ruu_issue_width*(data_width + pow2(opcode_length));
  num_piperegs += ruu_issue_width*(data_width + pow2(opcode_length));

  /* assume 50% extra in control signals (rule of thumb) */
  num_piperegs = num_piperegs * 1.5;

  pipereg_clockcap = num_piperegs * 4*gatecap(10.0,0);

  /* estimate based on 3% of die being in clock metal */
  Cline2 = Cmetal * (.03 * die_length * die_length/BitlineSpacing) * 1e6 * 1e6;

  /* another estimate */
#if 1
  clocklinelength = die_length*(.5 + 4 * (.25 + 2*(.25) + 4 * (.125)));
#else
  if ( die_length > 0.072) {
    /* Better htree estimation for big area factors */
    double stopAt  = 0.125;
    double section = die_length / 0.018;
    double total   = 0.5;
    double factor  = 0.5;
    while( (section * factor) >= stopAt ) {
      printf("new total = %g\n", total);
      total  = total + 2*total;
      factor = factor/4;
    }
    clocklinelength = die_length*total;
    printf("total = %g vs %g (section=%g)\n", total, (.5 + 4 * (.25 + 2*(.25) + 4 * (.125))), section);
  }else{
    clocklinelength = die_length*(.5 + 4 * (.25 + 2*(.25) + 4 * (.125)));
  }
#endif

#if 1
  /* Original model does not include the 3% of die being clock metal */
  Cline = 20 * Cmetal * (clocklinelength) * 1e6;
#else
  /* Use 2% metal lines are going to clock */
  Cline = 20 * Cmetal * (clocklinelength) * 1e6 + Cline2/1.5;
#endif

#if 0
  global_buffercap = 12*gatecap(1000.0,10.0)+16*gatecap(200,10.0)+16*8*2*gatecap(100.0,10.00) + 2*gatecap(.29*1e6,10.0);
#else
  /* The buffercap depends on the clock/area */
  global_buffercap = 12*gatecap(1000.0,10.0)+16*gatecap(200,10.0)+16*8*2*gatecap(100.0,10.00) + 2*gatecap(.29*1e6,10.0);
  global_buffercap *= die_length/0.018; /* This model is based on alpha with 0.018x0.018 area */
#endif
  /* global_clockcap is computed within each array structure for pre-charge tx's*/
  Ctotal = Cline+global_clockcap+pipereg_clockcap+global_buffercap;

  if(verbose)
    fprintf(stderr,"num_piperegs == %f\n",num_piperegs);

  /* add I_ADD Clockcap and F_ADD Clockcap */
  Clockpower = Ctotal*Powerfactor + res_ialu*I_ADD_CLOCK + res_fpalu*F_ADD_CLOCK;

  fprintf(stderr,"Global Clock Power: %g\n",Clockpower);

  if(verbose) {
    fprintf(stderr," Global Metal Lines   (W): %g\n",Cline*Powerfactor);
    /*    fprintf(stderr," Global Metal Lines (3%%) (W): %g\n",Cline2*Powerfactor); */
    fprintf(stderr," Global Clock Buffers (W): %g (factor %g)\n",global_buffercap*Powerfactor,die_length/0.018);
    fprintf(stderr," Global Clock Cap (Explicit) (W): %g\n"
	    ,global_clockcap*Powerfactor + res_ialu*I_ADD_CLOCK + res_fpalu*F_ADD_CLOCK);
    fprintf(stderr," Global ALUs (Explicit) (W): %g\n"
	    ,res_ialu*I_ADD_CLOCK + res_fpalu*F_ADD_CLOCK);

    fprintf(stderr," Global Clock Cap (Implicit) (W): %g\n",pipereg_clockcap*Powerfactor);
  }

  return(Clockpower);
}

/* very rough global clock power estimates */
double global_clockpower(double die_length)
{

  double clocklinelength;
  double Cline,Cline2,Ctotal;
  double global_buffercap = 0;

  Cline2 = Cmetal * (.03 * die_length * die_length/BitlineSpacing) * 1e6 * 1e6;

  clocklinelength = die_length*(.5 + 4 * (.25 + 2*(.25) + 4 * (.125)));
  Cline = 20 * Cmetal * (clocklinelength) * 1e6;
  global_buffercap = 12*gatecap(1000.0,10.0)+16*gatecap(200,10.0)+16*8*2*gatecap(100.0,10.00) + 2*gatecap(.29*1e6,10.0);
  Ctotal = Cline+global_buffercap;

  if(verbose) {
    fprintf(stderr,"Global Clock Power: %g\n",Ctotal*Powerfactor);
    fprintf(stderr," Global Metal Lines   (W): %g\n",Cline*Powerfactor);
    fprintf(stderr," Global Metal Lines (3%%) (W): %g\n",Cline2*Powerfactor);
    fprintf(stderr," Global Clock Buffers (W): %g\n",global_buffercap*Powerfactor);
  }

  return(Ctotal*Powerfactor);

}


double compute_resultbus_power()
{
  double Ctotal, Cline;

  double regfile_height;

  /* compute size of result bus tags */
  int npreg_width = (int)ceil(logtwo((double)RUU_size));

  Ctotal=0;

  regfile_height = RUU_size * (RegCellHeight + 
			       WordlineSpacing * 3 * ruu_issue_width); 

  /* assume num alu's == ialu  (FIXME: generate a more detailed result bus network model*/
  Cline = Cmetal * (regfile_height + .5 * res_ialu * 3200.0 * LSCALE);

  /* or use result bus length measured from 21264 die photo */
  /*  Cline = Cmetal * 3.3*1000;*/

  /* Assume ruu_issue_width result busses -- power can be scaled linearly
     for number of result busses (scale by writeback_access) */
  Ctotal += 2.0 * (data_width + npreg_width) * (ruu_issue_width)* Cline;

#ifdef STATIC_AF
  return(Ctotal*Powerfactor*AF);
#else
  return(Ctotal*Powerfactor);
#endif
  
}
bpower_result_type calculate_bpred_power(){
  double predeclength, wordlinelength, bitlinelength;
  double rasPower, lpPower, metaPower, bimodPower, btbPower;
  int ndwl, ndbl, nspd, ntwl, ntbl, ntspd, c,b,a,cache, rowsb, colsb;

  /* these variables are needed to use Cacti to auto-size cache arrays 
     (for optimal delay) */
  time_result_type time_result;
  time_parameter_type time_parameters;
  bpower_result_type btt;
  /* used to autosize other structures, like bpred tables */
  int scale_factor;

 /*******************************************************************************
  * local predict power
  *******************************************************************************/

  /* Load cache values into what cacti is expecting */
  time_parameters.cache_size = btb_config[0] * (data_width/8) * btb_config[1]; /* C */
  time_parameters.block_size = (data_width/8); /* B */
  time_parameters.associativity = btb_config[1]; /* A */
  time_parameters.number_of_sets = btb_config[0]; /* C/(B*A) */

  /* have Cacti compute optimal cache config */
  wattch_calculate_time(&time_result,&time_parameters);

  /* extract Cacti results */
  ndwl=time_result.best_Ndwl;
  ndbl=time_result.best_Ndbl;
  nspd=time_result.best_Nspd;
  ntwl=time_result.best_Ntwl;
  ntbl=time_result.best_Ntbl;
  ntspd=time_result.best_Ntspd;
  c = time_parameters.cache_size;
  b = time_parameters.block_size;
  a = time_parameters.associativity; 

  cache=1;

  /* Figure out how many rows/cols there are now */
  rowsb = c/(b*a*ndbl*nspd);
  colsb = 8*b*a*nspd/ndwl;

  if(verbose) {
    fprintf(stderr,"%d KB %d-way btb (%d-byte block size):\n",c,a,b);
    fprintf(stderr,"ndwl == %d, ndbl == %d, nspd == %d\n",ndwl,ndbl,nspd);
    fprintf(stderr,"%d sets of %d rows x %d cols\n",ndwl*ndbl,rowsb,colsb);
  }

  predeclength = rowsb * (RegCellHeight + WordlineSpacing);
  wordlinelength = colsb *  (RegCellWidth + BitlineSpacing);
  bitlinelength = rowsb * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"btb power stats\n");
  btbPower = ndwl*ndbl*(array_decoder_power(rowsb,colsb,predeclength,1,1,cache) + array_wordline_power(rowsb,colsb,wordlinelength,1,1,cache) + array_bitline_power(rowsb,colsb,bitlinelength,1,1,cache) + senseamp_power(colsb));

 /*******************************************************************************
  * local predict power
  *******************************************************************************/
  cache=1;

  scale_factor = squarify(twolev_config[0],twolev_config[2]);
  predeclength = (twolev_config[0] / scale_factor)* (RegCellHeight + WordlineSpacing);
  wordlinelength = twolev_config[2] * scale_factor *  (RegCellWidth + BitlineSpacing);
  bitlinelength = (twolev_config[0] / scale_factor) * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"local predict power stats\n");

  lpPower = array_decoder_power(twolev_config[0]/scale_factor,twolev_config[2]*scale_factor,predeclength,1,1,cache) + array_wordline_power(twolev_config[0]/scale_factor,twolev_config[2]*scale_factor,wordlinelength,1,1,cache) + array_bitline_power(twolev_config[0]/scale_factor,twolev_config[2]*scale_factor,bitlinelength,1,1,cache) + senseamp_power(twolev_config[2]*scale_factor);

  scale_factor = squarify(twolev_config[1],3);

  predeclength = (twolev_config[1] / scale_factor)* (RegCellHeight + WordlineSpacing);
  wordlinelength = 3 * scale_factor *  (RegCellWidth + BitlineSpacing);
  bitlinelength = (twolev_config[1] / scale_factor) * (RegCellHeight + WordlineSpacing);


  if(verbose)
    fprintf(stderr,"local predict power stats\n");
  lpPower += array_decoder_power(twolev_config[1]/scale_factor,3*scale_factor,predeclength,1,1,cache) + array_wordline_power(twolev_config[1]/scale_factor,3*scale_factor,wordlinelength,1,1,cache) + array_bitline_power(twolev_config[1]/scale_factor,3*scale_factor,bitlinelength,1,1,cache) + senseamp_power(3*scale_factor);

  /*******************************************************************************
   * bimod predictor power
   *******************************************************************************/

  if(verbose)
    fprintf(stderr,"bimod_config[0] == %d\n",bimod_config[0]);

  scale_factor = squarify(bimod_config[0],2);

  predeclength = bimod_config[0]/scale_factor * (RegCellHeight + WordlineSpacing);
  wordlinelength = 2*scale_factor *  (RegCellWidth + BitlineSpacing);
  bitlinelength = bimod_config[0]/scale_factor * (RegCellHeight + WordlineSpacing);


  if(verbose)
    fprintf(stderr,"bimod predict power stats\n");
  bimodPower = array_decoder_power(bimod_config[0]/scale_factor,2*scale_factor,predeclength,1,1,cache) + array_wordline_power(bimod_config[0]/scale_factor,2*scale_factor,wordlinelength,1,1,cache) + array_bitline_power(bimod_config[0]/scale_factor,2*scale_factor,bitlinelength,1,1,cache) + senseamp_power(2*scale_factor);

  /*******************************************************************************
   * meta predictor power
   *******************************************************************************/
  scale_factor = squarify(comb_config[0],2);

  predeclength = comb_config[0]/scale_factor * (RegCellHeight + WordlineSpacing);
  wordlinelength = 2*scale_factor *  (RegCellWidth + BitlineSpacing);
  bitlinelength = comb_config[0]/scale_factor * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"meta predict power stats\n");
  metaPower = array_decoder_power(comb_config[0]/scale_factor,2*scale_factor,predeclength,1,1,cache) + array_wordline_power(comb_config[0]/scale_factor,2*scale_factor,wordlinelength,1,1,cache) + array_bitline_power(comb_config[0]/scale_factor,2*scale_factor,bitlinelength,1,1,cache) + senseamp_power(2*scale_factor);


  /*******************************************************************************
   * ras power
   *******************************************************************************/
  if(verbose)
    fprintf(stderr,"RAS predict power stats\n");
  rasPower = simple_array_power(ras_size,data_width,1,1,0);

  // fill in the values
  if(bp_type == ORACLE){
    btt.rasPower = 0.0 ;
    btt.btbPower = 0.0 ;
    btt.bpredPower = 0.0 ;
  }
  else{
    btt.rasPower = rasPower ;
    btt.btbPower = btbPower ;
    btt.bpredPower = 0.0 ;
  }
  
  // bpred power for individual predictors
  if(bp_type == BIMOD){
    btt.bpredPower = bimodPower ;  
  }
  else if(bp_type == TWOLEV){
    btt.bpredPower = lpPower ;
  }
  else if(bp_type == COMB){
    btt.bpredPower = lpPower + bimodPower + metaPower ;
  }

  // crossover scaling
  btt.rasPower *= crossover_scaling ;
  btt.btbPower *= crossover_scaling ;
  btt.bpredPower *= crossover_scaling ;
  return btt ;
}

void calculate_power(power)
     power_result_type *power;
{
  bpower_result_type btt;
  double clockpower;
  double predeclength, wordlinelength, bitlinelength;
  int ndwl, ndbl, nspd, ntwl, ntbl, ntspd, c,b,a,cache, rowsb, colsb;
  int trowsb, tcolsb, tagsize, robwidth;
  int va_size = 48;

  int npreg_width = (int)ceil(logtwo((double)RUU_size));

  /* these variables are needed to use Cacti to auto-size cache arrays 
     (for optimal delay) */
  time_result_type time_result;
  time_parameter_type time_parameters;

  /* used to autosize other structures, like bpred tables */
  int scale_factor;

  global_clockcap = 0;

  cache=0;


  /* FIXME: ALU power is a simple constant, it would be better
     to include bit AFs and have different numbers for different
     types of operations */
  power->ialu_power = res_ialu * I_ADD;
  power->falu_power = res_fpalu * F_ADD;

  nvreg_width = (int)ceil(logtwo((double)MD_NUM_IREGS));
  npreg_width = (int)ceil(logtwo((double)RUU_size));


  /* RAT has shadow bits stored in each cell, this makes the
     cell size larger than normal array structures, so we must
     compute it here */

  predeclength = MD_NUM_IREGS * 
    (RatCellHeight + 3 * ruu_decode_width * WordlineSpacing);

  wordlinelength = npreg_width * 
    (RatCellWidth + 
     6 * ruu_decode_width * BitlineSpacing + 
     RatShiftRegWidth*RatNumShift);

  bitlinelength = MD_NUM_IREGS * (RatCellHeight + 3 * ruu_decode_width * WordlineSpacing);

  if(verbose)
    fprintf(stderr,"rat power stats\n");
  power->rat_decoder = array_decoder_power(MD_NUM_IREGS,npreg_width,predeclength,2*ruu_decode_width,ruu_decode_width,cache);
  power->rat_wordline = array_wordline_power(MD_NUM_IREGS,npreg_width,wordlinelength,2*ruu_decode_width,ruu_decode_width,cache);
  power->rat_bitline = array_bitline_power(MD_NUM_IREGS,npreg_width,bitlinelength,2*ruu_decode_width,ruu_decode_width,cache);
  power->rat_senseamp = 0;

  power->dcl_compare = dcl_compare_power(nvreg_width);
  power->dcl_pencode = 0;
  power->inst_decoder_power = ruu_decode_width * simple_array_decoder_power(opcode_length,1,1,1,cache);
  power->wakeup_tagdrive =cam_tagdrive(RUU_size,npreg_width,ruu_issue_width,ruu_issue_width);
  power->wakeup_tagmatch =cam_tagmatch(RUU_size,npreg_width,ruu_issue_width,ruu_issue_width);
  power->wakeup_ormatch =0; 

  power->selection = selection_power(RUU_size);


  predeclength = MD_NUM_IREGS * (RegCellHeight + 3 * ruu_issue_width * WordlineSpacing);

  wordlinelength = data_width * 
    (RegCellWidth + 
     6 * ruu_issue_width * BitlineSpacing);

  bitlinelength = MD_NUM_IREGS * (RegCellHeight + 3 * ruu_issue_width * WordlineSpacing);

  if(verbose)
    fprintf(stderr,"regfile power stats\n");

  power->regfile_decoder = array_decoder_power(REG_size,data_width,predeclength,2*ruu_issue_width,ruu_issue_width,cache);
  power->regfile_wordline = array_wordline_power(REG_size,data_width,wordlinelength,2*ruu_issue_width,ruu_issue_width,cache);
  power->regfile_bitline = array_bitline_power(REG_size,data_width,bitlinelength,2*ruu_issue_width,ruu_issue_width,cache);
  power->regfile_senseamp =0;

  predeclength = RUU_size * (RegCellHeight + 3 * ruu_issue_width * WordlineSpacing);

  wordlinelength = data_width * 
    (RegCellWidth + 
     6 * ruu_issue_width * BitlineSpacing);

  bitlinelength = RUU_size * (RegCellHeight + 3 * ruu_issue_width * WordlineSpacing);

  if(verbose)
    fprintf(stderr,"res station power stats\n");
  power->rs_decoder = array_decoder_power(RUU_size,data_width,predeclength,2*ruu_issue_width,ruu_issue_width,cache);
  power->rs_wordline = array_wordline_power(RUU_size,data_width,wordlinelength,2*ruu_issue_width,ruu_issue_width,cache);
  power->rs_bitline = array_bitline_power(RUU_size,data_width,bitlinelength,2*ruu_issue_width,ruu_issue_width,cache);
  /* no senseamps in reg file structures (only caches) */
  power->rs_senseamp =0;

  /* addresses go into ldq tag's */
  power->ldq_wakeup_tagdrive =cam_tagdrive(LDQ_size,data_width,res_memport,res_memport);
  power->ldq_wakeup_tagmatch =cam_tagmatch(LDQ_size,data_width,res_memport,res_memport);
  power->ldq_wakeup_ormatch =0; 

  power->stq_wakeup_tagdrive =cam_tagdrive(STQ_size,data_width,res_memport,res_memport);
  power->stq_wakeup_tagmatch =cam_tagmatch(STQ_size,data_width,res_memport,res_memport);
  power->stq_wakeup_ormatch =0; 

  wordlinelength = data_width * 
    (RegCellWidth + 
     4 * res_memport * BitlineSpacing);

  bitlinelength = RUU_size * (RegCellHeight + 4 * res_memport * WordlineSpacing);

  /* rs's hold data */
  if(verbose)
    fprintf(stderr,"LD/ST Queue station power stats\n");
  power->ldq_rs_decoder = array_decoder_power(LDQ_size,data_width,predeclength,res_memport,res_memport,cache);
  power->ldq_rs_wordline = array_wordline_power(LDQ_size,data_width,wordlinelength,res_memport,res_memport,cache);
  power->ldq_rs_bitline = array_bitline_power(LDQ_size,data_width,bitlinelength,res_memport,res_memport,cache);
  power->ldq_rs_senseamp =0;

  power->stq_rs_decoder = array_decoder_power(STQ_size,data_width,predeclength,res_memport,res_memport,cache);
  power->stq_rs_wordline = array_wordline_power(STQ_size,data_width,wordlinelength,res_memport,res_memport,cache);
  power->stq_rs_bitline = array_bitline_power(STQ_size,data_width,bitlinelength,res_memport,res_memport,cache);
  power->stq_rs_senseamp =0;

  power->resultbus = compute_resultbus_power();

  btt = calculate_bpred_power();
  /*  branch predictor power */  
  power->btb = btt.btbPower ;
  power->ras = btt.rasPower ;
  power->bpred_power = btt.bpredPower ;

  /**********************************************************
  ** rob power
  ***********************************************************/
  /* The ROB, does not need to have multiple ports FIFO Structure */
  power->reorder_power = simple_array_power(rob_size,2*data_width+8,1,1,0);
  /***************************** dtlb *********************************/
  
  tagsize = va_size - ((int)logtwo(cache_dl1->nsets) + (int)logtwo(cache_dl1->bsize));

  if(verbose)
    fprintf(stderr,"dtlb predict power stats\n");
  power->dtlb = res_memport*(cam_array(dtlb->nsets, va_size - (int)logtwo((double)dtlb->bsize),1,1) + simple_array_power(dtlb->nsets,tagsize,1,1,cache));

  tagsize = va_size - ((int)logtwo(cache_il1->nsets) + (int)logtwo(cache_il1->bsize));

  predeclength = itlb->nsets * (RegCellHeight + WordlineSpacing);
  wordlinelength = logtwo((double)itlb->bsize) * (RegCellWidth + BitlineSpacing);
  bitlinelength = itlb->nsets * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"itlb predict power stats\n");
  power->itlb = cam_array(itlb->nsets, va_size - (int)logtwo((double)itlb->bsize),1,1) + simple_array_power(itlb->nsets,tagsize,1,1,cache);


  cache=1;

  time_parameters.cache_size = cache_il1->nsets * cache_il1->bsize * cache_il1->assoc; /* C */
  time_parameters.block_size = cache_il1->bsize; /* B */
  time_parameters.associativity = cache_il1->assoc; /* A */
  time_parameters.number_of_sets = cache_il1->nsets; /* C/(B*A) */

  wattch_calculate_time(&time_result,&time_parameters);

  ndwl=time_result.best_Ndwl;
  ndbl=time_result.best_Ndbl;
  nspd=time_result.best_Nspd;
  ntwl=time_result.best_Ntwl;
  ntbl=time_result.best_Ntbl;
  ntspd=time_result.best_Ntspd;

  c = time_parameters.cache_size;
  b = time_parameters.block_size;
  a = time_parameters.associativity;

  rowsb = c/(b*a*ndbl*nspd);
  colsb = 8*b*a*nspd/ndwl;

  tagsize = va_size - ((int)logtwo(cache_il1->nsets) + (int)logtwo(cache_il1->bsize));
  trowsb = c/(b*a*ntbl*ntspd);
  tcolsb = a * (tagsize + 1 + 6) * ntspd/ntwl;
 
  if(verbose) {
    fprintf(stderr,"%d KB %d-way cache (%d-byte block size):\n",c,a,b);
    fprintf(stderr,"ndwl == %d, ndbl == %d, nspd == %d\n",ndwl,ndbl,nspd);
    fprintf(stderr,"%d sets of %d rows x %d cols\n",ndwl*ndbl,rowsb,colsb);
    fprintf(stderr,"tagsize == %d\n",tagsize);
  }

  predeclength = rowsb * (RegCellHeight + WordlineSpacing);
  wordlinelength = colsb *  (RegCellWidth + BitlineSpacing);
  bitlinelength = rowsb * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"icache power stats\n");
  power->icache_decoder = ndwl*ndbl*array_decoder_power(rowsb,colsb,predeclength,1,1,cache);
  power->icache_wordline = ndwl*ndbl*array_wordline_power(rowsb,colsb,wordlinelength,1,1,cache);
  power->icache_bitline = ndwl*ndbl*array_bitline_power(rowsb,colsb,bitlinelength,1,1,cache);
  power->icache_senseamp = ndwl*ndbl*senseamp_power(colsb);
  power->icache_tagarray = ntwl*ntbl*(simple_array_power(trowsb,tcolsb,1,1,cache));

  power->icache_power = power->icache_decoder + power->icache_wordline + power->icache_bitline + power->icache_senseamp + power->icache_tagarray;

  time_parameters.cache_size = cache_dl1->nsets * cache_dl1->bsize * cache_dl1->assoc; /* C */
  time_parameters.block_size = cache_dl1->bsize; /* B */
  time_parameters.associativity = cache_dl1->assoc; /* A */
  time_parameters.number_of_sets = cache_dl1->nsets; /* C/(B*A) */

  wattch_calculate_time(&time_result,&time_parameters);

  ndwl=time_result.best_Ndwl;
  ndbl=time_result.best_Ndbl;
  nspd=time_result.best_Nspd;
  ntwl=time_result.best_Ntwl;
  ntbl=time_result.best_Ntbl;
  ntspd=time_result.best_Ntspd;
  c = time_parameters.cache_size;
  b = time_parameters.block_size;
  a = time_parameters.associativity; 

  cache=1;

  rowsb = c/(b*a*ndbl*nspd);
  colsb = 8*b*a*nspd/ndwl;

  tagsize = va_size - ((int)logtwo(cache_dl1->nsets) + (int)logtwo(cache_dl1->bsize));
  trowsb = c/(b*a*ntbl*ntspd);
  tcolsb = a * (tagsize + 1 + 6) * ntspd/ntwl;

  if(verbose) {
    fprintf(stderr,"%d KB %d-way cache (%d-byte block size):\n",c,a,b);
    fprintf(stderr,"ndwl == %d, ndbl == %d, nspd == %d\n",ndwl,ndbl,nspd);
    fprintf(stderr,"%d sets of %d rows x %d cols\n",ndwl*ndbl,rowsb,colsb);
    fprintf(stderr,"tagsize == %d\n",tagsize);

    fprintf(stderr,"\nntwl == %d, ntbl == %d, ntspd == %d\n",ntwl,ntbl,ntspd);
    fprintf(stderr,"%d sets of %d rows x %d cols\n",ntwl*ntbl,trowsb,tcolsb);
  }

  predeclength = rowsb * (RegCellHeight + WordlineSpacing);
  wordlinelength = colsb *  (RegCellWidth + BitlineSpacing);
  bitlinelength = rowsb * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"dcache power stats\n");
  power->dcache_decoder = res_memport*ndwl*ndbl*array_decoder_power(rowsb,colsb,predeclength,1,1,cache);
  power->dcache_wordline = res_memport*ndwl*ndbl*array_wordline_power(rowsb,colsb,wordlinelength,1,1,cache);
  power->dcache_bitline = res_memport*ndwl*ndbl*array_bitline_power(rowsb,colsb,bitlinelength,1,1,cache);
  power->dcache_senseamp = res_memport*ndwl*ndbl*senseamp_power(colsb);
  power->dcache_tagarray = res_memport*ntwl*ntbl*(simple_array_power(trowsb,tcolsb,1,1,cache));

  power->dcache_power = power->dcache_decoder + power->dcache_wordline + power->dcache_bitline + power->dcache_senseamp + power->dcache_tagarray;

  clockpower = total_clockpower(dieLenght);
  power->clock_power = clockpower;
  if(verbose) {
    fprintf(stderr,"result bus power == %f\n",power->resultbus);
    fprintf(stderr,"global clock power == %f\n",clockpower);
  }

  time_parameters.cache_size = cache_dl2->nsets * cache_dl2->bsize * cache_dl2->assoc; /* C */
  time_parameters.block_size = cache_dl2->bsize; /* B */
  time_parameters.associativity = cache_dl2->assoc; /* A */
  time_parameters.number_of_sets = cache_dl2->nsets; /* C/(B*A) */

  wattch_calculate_time(&time_result,&time_parameters);

  ndwl=time_result.best_Ndwl;
  ndbl=time_result.best_Ndbl;
  nspd=time_result.best_Nspd;
  ntwl=time_result.best_Ntwl;
  ntbl=time_result.best_Ntbl;
  ntspd=time_result.best_Ntspd;
  c = time_parameters.cache_size;
  b = time_parameters.block_size;
  a = time_parameters.associativity;

  rowsb = c/(b*a*ndbl*nspd);
  colsb = 8*b*a*nspd/ndwl;

  tagsize = va_size - ((int)logtwo(cache_dl2->nsets) + (int)logtwo(cache_dl2->bsize));
  trowsb = c/(b*a*ntbl*ntspd);
  tcolsb = a * (tagsize + 1 + 6) * ntspd/ntwl;

  if(verbose) {
    fprintf(stderr,"%d KB %d-way cache (%d-byte block size):\n",c,a,b);
    fprintf(stderr,"ndwl == %d, ndbl == %d, nspd == %d\n",ndwl,ndbl,nspd);
    fprintf(stderr,"%d sets of %d rows x %d cols\n",ndwl*ndbl,rowsb,colsb);
    fprintf(stderr,"tagsize == %d\n",tagsize);
  }

  predeclength = rowsb * (RegCellHeight + WordlineSpacing);
  wordlinelength = colsb *  (RegCellWidth + BitlineSpacing);
  bitlinelength = rowsb * (RegCellHeight + WordlineSpacing);

  if(verbose)
    fprintf(stderr,"dcache2 power stats\n");
  power->dcache2_decoder = array_decoder_power(rowsb,colsb,predeclength,1,1,cache);
  power->dcache2_wordline = array_wordline_power(rowsb,colsb,wordlinelength,1,1,cache);
  power->dcache2_bitline = array_bitline_power(rowsb,colsb,bitlinelength,1,1,cache);
  power->dcache2_senseamp = senseamp_power(colsb);
  power->dcache2_tagarray = simple_array_power(trowsb,tcolsb,1,1,cache);

  power->dcache2_power = power->dcache2_decoder + power->dcache2_wordline + power->dcache2_bitline + power->dcache2_senseamp + power->dcache2_tagarray;

  power->rat_decoder *= crossover_scaling;
  power->rat_wordline *= crossover_scaling;
  power->rat_bitline *= crossover_scaling;

  power->dcl_compare *= crossover_scaling;
  power->dcl_pencode *= crossover_scaling;
  power->inst_decoder_power *= crossover_scaling;
  power->wakeup_tagdrive *= crossover_scaling;
  power->wakeup_tagmatch *= crossover_scaling;
  power->wakeup_ormatch *= crossover_scaling;

  power->selection *= crossover_scaling;

  power->regfile_decoder *= crossover_scaling;
  power->regfile_wordline *= crossover_scaling;
  power->regfile_bitline *= crossover_scaling;
  power->regfile_senseamp *= crossover_scaling;

  power->rs_decoder *= crossover_scaling;
  power->rs_wordline *= crossover_scaling;
  power->rs_bitline *= crossover_scaling;
  power->rs_senseamp *= crossover_scaling;

  power->ldq_wakeup_tagdrive *= crossover_scaling;
  power->ldq_wakeup_tagmatch *= crossover_scaling;

  power->stq_wakeup_tagdrive *= crossover_scaling;
  power->stq_wakeup_tagmatch *= crossover_scaling;

  power->ldq_rs_decoder *= crossover_scaling;
  power->ldq_rs_wordline *= crossover_scaling;
  power->ldq_rs_bitline *= crossover_scaling;
  power->ldq_rs_senseamp *= crossover_scaling;

  power->stq_rs_decoder *= crossover_scaling;
  power->stq_rs_wordline *= crossover_scaling;
  power->stq_rs_bitline *= crossover_scaling;
  power->stq_rs_senseamp *= crossover_scaling;
 
  power->resultbus *= crossover_scaling;

  power->dtlb *= crossover_scaling;

  power->itlb *= crossover_scaling;

  power->icache_decoder *= crossover_scaling;
  power->icache_wordline*= crossover_scaling;
  power->icache_bitline *= crossover_scaling;
  power->icache_senseamp*= crossover_scaling;
  power->icache_tagarray*= crossover_scaling;

  power->icache_power *= crossover_scaling;

  power->dcache_decoder *= crossover_scaling;
  power->dcache_wordline *= crossover_scaling;
  power->dcache_bitline *= crossover_scaling;
  power->dcache_senseamp *= crossover_scaling;
  power->dcache_tagarray *= crossover_scaling;

  power->dcache_power *= crossover_scaling;
  
  power->clock_power *= crossover_scaling;

  power->dcache2_decoder *= crossover_scaling;
  power->dcache2_wordline *= crossover_scaling;
  power->dcache2_bitline *= crossover_scaling;
  power->dcache2_senseamp *= crossover_scaling;
  power->dcache2_tagarray *= crossover_scaling;

  power->dcache2_power *= crossover_scaling;

  power->reorder_power *= crossover_scaling;

  power->total_power = power->bpred_power  +
    power->btb +
    power->rat_decoder + power->rat_wordline + 
    power->rat_bitline + power->rat_senseamp + 
    power->dcl_compare + power->dcl_pencode + 
    power->inst_decoder_power +
    power->wakeup_tagdrive + power->wakeup_tagmatch + 
    power->selection +
    power->regfile_decoder + power->regfile_wordline + 
    power->regfile_bitline + power->regfile_senseamp +  
    power->rs_decoder + power->rs_wordline +
    power->rs_bitline + power->rs_senseamp + 

    power->ldq_wakeup_tagdrive + power->ldq_wakeup_tagmatch +
    power->ldq_rs_decoder + power->ldq_rs_wordline +
    power->ldq_rs_bitline + power->ldq_rs_senseamp +

    power->stq_wakeup_tagdrive + power->stq_wakeup_tagmatch +
    power->stq_rs_decoder + power->stq_rs_wordline +
    power->stq_rs_bitline + power->stq_rs_senseamp +

    power->resultbus +
    power->clock_power +
    power->icache_power + 
    power->itlb + 
    power->dcache_power + 
    power->dtlb + 
    power->dcache2_power+ power->reorder_power ;

  power->total_power_nodcache2 =power->bpred_power +
    power->btb +
    power->rat_decoder + power->rat_wordline + 
    power->rat_bitline + power->rat_senseamp + 
    power->dcl_compare + power->dcl_pencode + 
    power->inst_decoder_power +
    power->wakeup_tagdrive + power->wakeup_tagmatch + 
    power->selection +
    power->regfile_decoder + power->regfile_wordline + 
    power->regfile_bitline + power->regfile_senseamp +  
    power->rs_decoder + power->rs_wordline +
    power->rs_bitline + power->rs_senseamp + 
    power->ldq_wakeup_tagdrive + power->ldq_wakeup_tagmatch +
    power->ldq_rs_decoder + power->ldq_rs_wordline +
    power->ldq_rs_bitline + power->ldq_rs_senseamp +

    power->stq_wakeup_tagdrive + power->stq_wakeup_tagmatch +
    power->stq_rs_decoder + power->stq_rs_wordline +
    power->stq_rs_bitline + power->stq_rs_senseamp +

    power->resultbus +
    power->clock_power +
    power->icache_power + 
    power->itlb + 
    power->dcache_power + 
    power->dtlb + 
    power->dcache2_power + power->reorder_power ;

  power->rat_power = power->rat_decoder + 
    power->rat_wordline + power->rat_bitline + power->rat_senseamp;

  power->dcl_power = power->dcl_compare + power->dcl_pencode;

  power->rename_power = power->rat_power + 
    power->dcl_power + 
    power->inst_decoder_power;

  power->wakeup_power = power->wakeup_tagdrive + power->wakeup_tagmatch + 
    power->wakeup_ormatch;

  power->rs_power = power->rs_decoder + 
    power->rs_wordline + power->rs_bitline + power->rs_senseamp;

  power->rs_power_nobit = power->rs_decoder + 
    power->rs_wordline + power->rs_senseamp;

  power->window_power = power->wakeup_power + power->rs_power + 
    power->selection;

  power->ldq_rs_power = power->ldq_rs_decoder + 
    power->ldq_rs_wordline + power->ldq_rs_bitline + 
    power->ldq_rs_senseamp;

  power->stq_rs_power = power->stq_rs_decoder + 
    power->stq_rs_wordline + power->stq_rs_bitline + 
    power->stq_rs_senseamp;

  power->ldq_rs_power_nobit = power->ldq_rs_decoder + 
    power->ldq_rs_wordline + power->ldq_rs_senseamp;

  power->stq_rs_power_nobit = power->stq_rs_decoder + 
    power->stq_rs_wordline + power->stq_rs_senseamp;
   
  power->ldq_wakeup_power = power->ldq_wakeup_tagdrive + power->ldq_wakeup_tagmatch;

  power->stq_wakeup_power = power->stq_wakeup_tagdrive + power->stq_wakeup_tagmatch;

  power->ldq_power = power->ldq_wakeup_power + power->ldq_rs_power;

  power->stq_power = power->stq_wakeup_power + power->stq_rs_power;

  power->regfile_power = power->regfile_decoder + 
    power->regfile_wordline + power->regfile_bitline + 
    power->regfile_senseamp;

  power->regfile_power_nobit = power->regfile_decoder + 
    power->regfile_wordline + power->regfile_senseamp;

}
