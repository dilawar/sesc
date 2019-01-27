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
#include <stdio.h>
#include <stdlib.h>

#include "cacti42_def.h"
#include "cacti42_areadef.h"
#include "cacti42_basic_circuit.h"

extern int force_tag, force_tag_size;//Added by Shyam

//v4.1: Earlier all the dimensions (length/width/thickness) of the transistors and wires
//were calculated for the 0.8 micron process and then scaled to the input techology. Now
//all dimensions are calculated directly for the input technology, so area no longer needs to
//be scaled to the input technology.

double
logtwo_area (double x)
{
  if (x <= 0)
    printf ("%e\n", x);
  return ((double) (log (x) / log (2.0)));
}

double calculate_area (area_type module_area,double techscaling_factor)
{
  return (module_area.height * module_area.width * (1 / techscaling_factor) *
				  (1 / techscaling_factor));
}

area_type
inverter_area (double Widthp,double Widthn)
{
  double Width_n, Width_p;
  area_type invarea;
  int foldp = 0, foldn = 0;
  if (Widthp > 10.0 / FUDGEFACTOR)
    {
      Widthp = Widthp / 2, foldp = 1;
    }
  if (Widthn > 10.0 / FUDGEFACTOR)
    {
      Widthn = Widthn / 2, foldn = 1;
    }
  invarea.height = Widthp + Widthn + Widthptondiff + 2 * Widthtrack;
  Width_n =
    (foldn) ? (3 * Widthcontact +
	       2 * (Wpoly + 2 * ptocontact)) : (2 * Widthcontact + Wpoly +
						2 * ptocontact);
  Width_p =
    (foldp) ? (3 * Widthcontact +
	       2 * (Wpoly + 2 * ptocontact)) : (2 * Widthcontact + Wpoly +
						2 * ptocontact);
  invarea.width = MAX (Width_n, Width_p);
  return (invarea);
}

area_type
subarraymem_area (int C,int B,int A,int Ndbl,int Ndwl,double Nspd,int RWP,int ERP,int EWP,int NSER,double techscaling_factor)	/* returns area of subarray */
{
  area_type memarea;
  int noof_rows, noof_colns;

  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //noof_rows = (C / (B * A * Ndbl * Nspd));
  //noof_colns = (8 * B * A * Nspd / Ndwl);
  noof_rows = (int)((C / (B * A * Ndbl * Nspd)) + EPSILON);
  noof_colns = (int)((8 * B * A * Nspd / Ndwl) + EPSILON);

  memarea.height = ceil((double)(noof_rows)/16.0)*stitch_ramv+(BitHeight1x1+Widthtrack*2*(RWP+ERP+EWP-1))*noof_rows;
  memarea.width  = noof_colns*(BitWidth1x1+(Widthtrack*2*(RWP+(ERP-NSER)+EWP-1)+Widthtrack*NSER));
	/* dt
	was : memarea.height = ceil((double)(noof_rows)/16.0)*stitch_ramv+(BitHeight16x2+2*Widthtrack*2*(RWP+ERP+EWP-1))*ceil((double)(noof_rows)/2.0);
		  memarea.width  = ceil((double)(noof_colns)/16.0)*(BitWidth16x2+16*(Widthtrack*2*(RWP+(ERP-NSER)+EWP-1)+Widthtrack*NSER));
	now : use single cell for width and height
	*/

  //area_all_dataramcells =
    //Ndwl * Ndbl * calculate_area (memarea, techscaling_factor) * CONVERT_TO_MMSQUARE;
  area_all_dataramcells = Ndwl * Ndbl * memarea.height * memarea.width * CONVERT_TO_MMSQUARE;
  return (memarea);
}

area_type
decodemem_row (int C,int B,int A,int Ndbl,double Nspd,int Ndwl,int RWP,int ERP,int EWP)	/* returns area of post decode */
{
  int noof_colns, numstack;
  double decodeNORwidth;
  double desiredrisetime, Cline, Rpdrive, psize, nsize;
  area_type decinv, worddriveinv, postdecodearea;

  /*
  How many bit columns do we have in each subarray?
  Since our basic unit is the byte, we have 8 bits to the byte.
  Each Block is made up of B bytes -> 8*B
  If we have associativity A, then all the ways are mapped to one wordline -> 8*B*A
  If we mapped more than one set to each wordline (if Nspd > 1) than the wordline is longer again -> 8*B*A*Nspd
  If we have subdivided the global wordline into segments (if Ndwl > 1) than the local wordline is shorter -> 8*B*A*Nspd/Ndwl
  */
  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //noof_colns = 8 * B * A * Nspd / Ndwl;
  noof_colns = (int) (8 * B * A * Nspd / Ndwl + EPSILON);
  desiredrisetime = krise * log ((double) (noof_colns)) / 2.0;
  Cline = (2 * Wmemcella * Leff * Cgatepass + Cwordmetal) * noof_colns;
  Rpdrive = desiredrisetime / (Cline * log (VSINV) * -1.0);
  psize = Rpchannelon / Rpdrive;
  if (psize > Wworddrivemax)
    {
      psize = Wworddrivemax;
    }
  numstack =
    (int)ceil ((1.0 / 3.0) * logtwo_area ((double)
		       ((double) C / (double) (B * A * Ndbl * Nspd))));
  if (numstack == 0)
    numstack = 1;
  if (numstack > 5)
    numstack = 5;
  switch (numstack)
    {
    case 1:
      decodeNORwidth = WidthNOR1;
      break;
    case 2:
      decodeNORwidth = WidthNOR2;
      break;
    case 3:
      decodeNORwidth = WidthNOR3;
      break;
    case 4:
      decodeNORwidth = WidthNOR4;
      break;
    case 5:
      decodeNORwidth = WidthNOR4;
      break;
    default:
      printf ("error:numstack=%d\n", numstack);
      printf ("Cacti does not support a series stack of %d transistors !\n",
	      numstack);
      exit(0);
      break;

    }
  nsize = psize * Wdecinvn / Wdecinvp;
  decinv = inverter_area (Wdecinvp, Wdecinvn);
  worddriveinv = inverter_area (psize, nsize);
  /*
	was: postdecodearea.height = (BitHeight16x2+2*Widthtrack*2*(RWP+ERP+EWP-1));
  */
  postdecodearea.height = (2*BitHeight1x1+2*Widthtrack*2*(RWP+ERP+EWP-1));
  postdecodearea.width =
    (decodeNORwidth + decinv.height + worddriveinv.height) * (RWP + ERP +
							      EWP);
  return (postdecodearea);
}

//v4.1: Making noof_rows double since the variable colns_datasubarray is function area
//was made double and is used as an argument corresponding to noof_rows in function calls
//to predecode_area
//area_type
//predecode_area (int noof_rows,int RWP,int ERP,int EWP)	/*returns the area of predecode */
area_type
predecode_area (double noof_rows,int RWP,int ERP,int EWP)
/* this puts the different predecode blocks for the different ports side by side and does not put them as an array or something */

{
  area_type predecode, predecode_temp;
  int N3to8;
  //v4.1: noof_rows can be less than 1 now since because Nspd can be a fraction. When
  //noof_rows is less than 1 making N3to8 1. This is not clean and needs to be fixed later.
  if(noof_rows < 1)
	N3to8 = 1;
  else
	//v4.1: using integer casting below
	//N3to8 = ceil ((1.0 / 3.0) * logtwo_area ((double) (noof_rows)));
      N3to8 = (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (noof_rows))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  switch (N3to8)
    {
    case 1:
      predecode_temp.height = Predec_height1;
      predecode_temp.width = Predec_width1;
      break;
    case 2:
      predecode_temp.height = Predec_height2;
      predecode_temp.width = Predec_width2;
      break;
    case 3:
      predecode_temp.height = Predec_height3;
      predecode_temp.width = Predec_width3;
      break;
    case 4:
      predecode_temp.height = Predec_height4;
      predecode_temp.width = Predec_width4;
      break;
    case 5:
      predecode_temp.height = Predec_height5;
      predecode_temp.width = Predec_width5;
      break;
    case 6:
      predecode_temp.height = Predec_height6;
      predecode_temp.width = Predec_width6;
      break;
    default:
      printf ("error:N3to8=%d\n", N3to8);
      exit (0);

    }

  predecode.height = predecode_temp.height;
  predecode.width = predecode_temp.width * (RWP + ERP + EWP);
  return (predecode);
}

//v4.1: Making noof_rows double since the variable colns_datasubarray is function area
//was made double and is used as an argument corresponding to noof_rows in function calls
//to postdecode_area
area_type
postdecode_area (int noof_rows,int RWP,int ERP,int EWP)
{
  //v4.1:	Making decodeNORwidth double which is what it should be
  //int numstack, decodeNORwidth;
  int numstack;
  double decodeNORwidth;
  area_type postdecode, decinverter;
  decinverter = inverter_area (Wdecinvp, Wdecinvn);
  //v4.1: noof_rows can be less than 1 now since because Nspd can be a fraction. When
  //noof_rows is less than 1 making N3to8 1. This is not clean and needs to be fixed later.
  if(noof_rows < 1)
	numstack = 1;
  else
    //v4.1: using integer casting below
	//numstack = ceil ((1.0 / 3.0) * logtwo_area ((double) (noof_rows)));
    numstack = (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (noof_rows))));
  if (numstack == 0)
    numstack = 1;
  if (numstack > 5)
    numstack = 5;
  switch (numstack)
    {
    case 1:
      decodeNORwidth = WidthNOR1;
      break;
    case 2:
      decodeNORwidth = WidthNOR2;
      break;
    case 3:
      decodeNORwidth = WidthNOR3;
      break;
    case 4:
      decodeNORwidth = WidthNOR4;
      break;
    case 5:
      decodeNORwidth = WidthNOR4;
      break;
    default:
      printf ("error:numstack=%d\n", numstack);
      printf ("Cacti does not support a series stack of %d transistors !\n",
	      numstack);
      exit (0);
      break;

    }
  postdecode.height =
    (BitHeight + Widthtrack * 2 * (RWP + ERP + EWP - 1)) * noof_rows;
  postdecode.width =
    (2 * decinverter.height + decodeNORwidth) * (RWP + ERP + EWP);
  return (postdecode);
}

area_type
colmux (int Ndbl,double Nspd,int RWP,int ERP,int EWP,int NSER)	/* gives the height of the colmux */
{
  area_type colmux_area;
  colmux_area.height =
    (2 * Wiso + 3 * (2 * Widthcontact + 1 / FUDGEFACTOR)) * (RWP + ERP + EWP);//Shyam: Need to understand what the +1 is for
  colmux_area.width =
    (BitWidth + Widthtrack * 2 * (RWP + (ERP - NSER) + EWP - 1) +
     Widthtrack * NSER);
  return (colmux_area);
}

area_type
precharge (int Ndbl,double Nspd,int RWP,int ERP,int EWP,int NSER)
{
  area_type precharge_area;
  if (Ndbl * Nspd > 1)
    {
      precharge_area.height =
	(Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact + 1 / FUDGEFACTOR) +
	 3 * Widthptondiff) * 0.5 * (RWP + EWP);
      precharge_area.width =
	2 * (BitWidth + Widthtrack * 2 * (RWP + (ERP - NSER) + EWP - 1) +
	     Widthtrack * NSER);
    }
  else
    {
      precharge_area.height =
	(Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact + 1 / FUDGEFACTOR) +
	 3 * Widthptondiff) * (RWP + EWP);
      precharge_area.width =
	BitWidth + Widthtrack * 2 * (RWP + (ERP - NSER) + EWP - 1) +
	Widthtrack * NSER;
    }
  return (precharge_area);
}

area_type
senseamp (int Ndbl,double Nspd,int RWP,int ERP,int EWP,int NSER)
{
  area_type senseamp_area;
  if (Ndbl * Nspd > 1)
    {
      senseamp_area.height = 0.5 * SenseampHeight * (RWP + ERP);
      senseamp_area.width =
	2 * (BitWidth + Widthtrack * 2 * (RWP + (ERP - NSER) + EWP - 1) +
	     Widthtrack * NSER);
    }
  else
    {
      senseamp_area.height = SenseampHeight * (RWP + ERP);
      senseamp_area.width =
	BitWidth + Widthtrack * 2 * (RWP + (ERP - NSER) + EWP - 1) +
	Widthtrack * NSER;
    }
  return (senseamp_area);
}

/* define OutdriveHeight OutdriveWidth DatainvHeight DatainvWidth */

area_type
subarraytag_area (int baddr,int C,int B,int A,int Ntdbl,int Ntdwl,int Ntspd,double NSubbanks,int RWP,int ERP,
		  int EWP,int NSER,double techscaling_factor)	/* returns area of subarray */
{
  area_type tagarea;
  int noof_rows, noof_colns, Tagbits;
  int conservative_NSER;

  conservative_NSER = 0;

  //Added by Shyam to make area model sensitive to "change tag" feature

  if(!force_tag) {
	    //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
        //the final int value is the correct one
		//Tagbits = ADDRESS_BITS + EXTRA_TAG_BITS-(int)logtwo((double)C)+(int)logtwo((double)A)-(int)(logtwo(NSubbanks));
		Tagbits = (int) (ADDRESS_BITS + EXTRA_TAG_BITS-(int)logtwo((double)C)+(int)logtwo((double)A)-(int)(logtwo(NSubbanks)) + EPSILON);
  }
  else {
	    Tagbits = force_tag_size;
	}

  //Commented by Shyam Tagbits =
    //baddr - (int) (logtwo_area ((double) (C))) +
    //(int) (logtwo_area ((double) (A))) + 2 - (int) (logtwo_area (NSubbanks));

  noof_rows = (C / (B * A * Ntdbl * Ntspd));
  noof_colns = (Tagbits * A * Ntspd / Ntdwl);
  tagarea.height = ceil((double)(noof_rows)/16.0)*stitch_ramv+(BitHeight1x1+Widthtrack*2*(RWP+ERP+EWP-1))*noof_rows;
  tagarea.width  = noof_colns*(BitWidth1x1+(Widthtrack*2*(RWP+(ERP-conservative_NSER)+EWP-1)+Widthtrack*conservative_NSER));
  /*
  was:
  tagarea.height = ceil((double)(noof_rows)/16.0)*stitch_ramv+(BitHeight16x2+2*Widthtrack*2*(RWP+ERP+EWP-1))*ceil((double)(noof_rows)/2.0);
  tagarea.width = ceil((double)(noof_colns)/16.0)*(BitWidth16x2+16*(Widthtrack*2*(RWP+(ERP-conservative_NSER)+EWP-1)+Widthtrack*conservative_NSER));
  now:
  using single cell for width and height
  */

  //area_all_tagramcells =
    //Ntdwl * Ntdbl * calculate_area (tagarea,
				    //techscaling_factor) * CONVERT_TO_MMSQUARE;
  area_all_tagramcells =  Ntdwl * Ntdbl * tagarea.height *tagarea.width * CONVERT_TO_MMSQUARE;
  return (tagarea);
}

area_type
decodetag_row (int baddr,int C,int B,int A,int Ntdbl,int Ntspd,int Ntdwl,double NSubbanks,int RWP,int ERP,int EWP)	/* returns area of post decode */
{
  int numstack, Tagbits;
  double decodeNORwidth;
  area_type decinv, worddriveinv, postdecodearea;

  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //Tagbits =
    //baddr - logtwo_area ((double) (C)) + logtwo_area ((double) (A)) + 2 -
    //logtwo_area ((double) (NSubbanks));
  Tagbits = (int) (baddr - logtwo_area ((double) (C)) + logtwo_area ((double) (A)) + 2 -
    logtwo_area ((double) (NSubbanks)) + EPSILON);

  //v4.1: using integer casting below
  //numstack =
    //ceil ((1.0 / 3.0) *
	  //logtwo_area ((double)
		       //((double) C / (double) (B * A * Ntdbl * Ntspd))));
  numstack =
    (int) (ceil ((1.0 / 3.0) *
	  logtwo_area ((double)
		       ((double) C / (double) (B * A * Ntdbl * Ntspd)))));
  if (numstack == 0)
    numstack = 1;
  if (numstack > 5)
    numstack = 5;
  switch (numstack)
    {
    case 1:
      decodeNORwidth = WidthNOR1;
      break;
    case 2:
      decodeNORwidth = WidthNOR2;
      break;
    case 3:
      decodeNORwidth = WidthNOR3;
      break;
    case 4:
      decodeNORwidth = WidthNOR4;
      break;
    case 5:
      decodeNORwidth = WidthNOR4;
      break;
    default:
      printf ("error:numstack=%d\n", numstack);
      printf ("Cacti does not support a series stack of %d transistors !\n",
	      numstack);
      exit (0);
      break;

    }
  decinv = inverter_area (Wdecinvp, Wdecinvn);
  worddriveinv = inverter_area (Wdecinvp, Wdecinvn);
  /*
  postdecodearea.height = (BitHeight16x2+2*Widthtrack*2*(RWP+ERP+EWP-1));
  */
  postdecodearea.height = (2*BitHeight1x1+2*Widthtrack*2*(RWP+ERP+EWP-1));
  postdecodearea.width =
    (decodeNORwidth + decinv.height + worddriveinv.height) * (RWP + ERP +
							      EWP);
  return (postdecodearea);
}

area_type
comparatorbit (int RWP,int ERP,int EWP)
{
  area_type compbit_area;
  compbit_area.width = 3 * Widthcontact + 2 * (3 * Wpoly + 2 * ptocontact);
  compbit_area.height = (Wcompn + 2 * (2 * Widthcontact + 1 / FUDGEFACTOR)) * (RWP + ERP);
  return (compbit_area);
}

area_type
muxdriverdecode (int B,int b0,int RWP,int ERP,int EWP)
{
  int noof_rows;
  area_type muxdrvdecode_area, predecode, postdecode;
  noof_rows = (8 * B) / b0;
  predecode = predecode_area (noof_rows, RWP, ERP, EWP);
  postdecode = postdecode_area (noof_rows, RWP, ERP, EWP);
  muxdrvdecode_area.width =
    predecode.height + postdecode.width + noof_rows * Widthtrack * (RWP +
								    ERP +
								    EWP);
  muxdrvdecode_area.height = MAX (predecode.width, postdecode.height);
  return (muxdrvdecode_area);
}

area_type
muxdrvsig (int A,int B,int b0)		/* generates the 8B/b0*A signals */
{
  int noof_rows;
  area_type outdrvsig_area;
  area_type muxdrvsig_area;
  noof_rows = (8 * B) / b0;
  //debug
  muxdrvsig_area.height = 0;
  muxdrvsig_area.width = 0;

  outdrvsig_area.height =
    0.5 * (WmuxdrvNORn + WmuxdrvNORp) + 9 * Widthcontact + 0.5 * (Wmuxdrv3n +
								  Wmuxdrv3p) +
    Widthptondiff + 3 * Widthcontact;
  outdrvsig_area.width =
    (3 * Widthcontact + 2 * (3 * Wpoly + 2 * ptocontact)) * noof_rows;
  switch (A)
    {
    case 1:
      muxdrvsig_area.height =
	outdrvsig_area.height + noof_rows * Widthtrack * 2 + A * Widthtrack;
      muxdrvsig_area.width =
	outdrvsig_area.width + noof_rows * Widthtrack + A * Widthtrack;
      break;
    case 2:
      muxdrvsig_area.height =
	outdrvsig_area.height * 2 + noof_rows * Widthtrack * 3 +
	A * Widthtrack;
      muxdrvsig_area.width =
	outdrvsig_area.width + noof_rows * Widthtrack + A * Widthtrack;
      break;
    case 4:
      muxdrvsig_area.height =
	outdrvsig_area.height * 2 + noof_rows * Widthtrack * 5 +
	A * Widthtrack;
      muxdrvsig_area.width =
	outdrvsig_area.width * 2 + noof_rows * Widthtrack + A * Widthtrack;
      break;
    case 8:
      muxdrvsig_area.height =
	outdrvsig_area.height * 2 + noof_rows * Widthtrack * 9 +
	A * Widthtrack;
      muxdrvsig_area.width =
	outdrvsig_area.width * 4 + noof_rows * Widthtrack + A * Widthtrack;
      break;
    case 16:
      muxdrvsig_area.height =
	outdrvsig_area.height * 4 + noof_rows * Widthtrack * 18 +
	A * Widthtrack;
      muxdrvsig_area.width =
	outdrvsig_area.width * 4 + noof_rows * Widthtrack + A * Widthtrack;
      break;
    case 32:
      muxdrvsig_area.height =
	outdrvsig_area.height * 4 + noof_rows * Widthtrack * 35 +
	2 * A * Widthtrack;
      muxdrvsig_area.width =
	2 * (outdrvsig_area.width * 4 + noof_rows * Widthtrack +
	     A * Widthtrack);
      break;
    default:
      printf ("error:Associativity=%d\n", A);
    }

  return (muxdrvsig_area);
}


area_type
datasubarray (int C,int B,int A,int Ndbl,int Ndwl,double Nspd,int RWP,int ERP,int EWP,int NSER,
	      double techscaling_factor)
{

  //area_type datasubarray_area, mem_area, postdecode_area, colmux_area,
    //precharge_area, senseamp_area, outdrv_area;
  area_type datasubarray_area, mem_area, postdecode_area, colmux_area,
    precharge_area, senseamp_area;
  mem_area =
    subarraymem_area (C, B, A, Ndbl, Ndwl, Nspd, RWP, ERP, EWP, NSER,
		      techscaling_factor);
  postdecode_area = decodemem_row (C, B, A, Ndbl, Nspd, Ndwl, RWP, ERP, EWP);
  colmux_area = colmux (Ndbl, Nspd, RWP, ERP, EWP, NSER);
  precharge_area = precharge (Ndbl, Nspd, RWP, ERP, EWP, NSER);
  senseamp_area = senseamp (Ndbl, Nspd, RWP, ERP, EWP, NSER);
  datasubarray_area.height =
    mem_area.height + colmux_area.height + precharge_area.height +
    senseamp_area.height + DatainvHeight * (RWP + EWP) +
    OutdriveHeight * (RWP + ERP);
  datasubarray_area.width = mem_area.width + postdecode_area.width;

  return (datasubarray_area);
}

area_type
datasubblock (int C,int B,int A,int Ndbl,int Ndwl,double Nspd,int SB,int b0,int RWP,int ERP,int EWP,int NSER,
	      double techscaling_factor)
{
  int N3to8;
  int colmuxtracks_rem, outrdrvtracks_rem, writeseltracks_rem;
  int SB_;
  double tracks_h, tracks_w;
  area_type datasubarray_area, datasubblock_area;
  SB_ = SB;
  if (SB_ == 0)
    {
      SB_ = 1;
    }
  colmuxtracks_rem =
    (Ndbl * Nspd >
     tracks_precharge_p) ? (Ndbl * Nspd - tracks_precharge_p) : 0;
  outrdrvtracks_rem =
    ((2 * B * A) / (b0) >
     tracks_outdrvselinv_p) ? ((2 * B * A) / (b0) -
			       tracks_outdrvselinv_p) : 0;
  writeseltracks_rem =
    ((2 * B * A) / (b0) >
     tracks_precharge_nx2) ? ((2 * B * A) / (b0) - tracks_precharge_nx2) : 0;
 //v4.1: using integer casting below
  //N3to8 =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd))));
 N3to8 =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd)))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  tracks_h =
    Widthtrack * (N3to8 * 8 * (RWP + ERP + EWP) +
		  (RWP + EWP) * colmuxtracks_rem + Ndbl * Nspd * ERP +
		  4 * outrdrvtracks_rem * (RWP + ERP) +
		  4 * writeseltracks_rem * (RWP + EWP) + (RWP + ERP +
							  EWP) * b0 / SB_);
  tracks_w = Widthtrack * (N3to8 * 8) * (RWP + ERP + EWP);
  datasubarray_area =
    datasubarray (C, B, A, Ndbl, Ndwl, Nspd, RWP, ERP, EWP, NSER,
		  techscaling_factor);
  datasubblock_area.height = 2 * datasubarray_area.height + tracks_h;
  datasubblock_area.width = 2 * datasubarray_area.width + tracks_w;

  return (datasubblock_area);
}

area_type
dataarray (int C,int B,int A,int Ndbl,int Ndwl,double Nspd,int b0,int RWP,int ERP,int EWP,int NSER,
	   double techscaling_factor)
{
  int SB, N3to8;
  area_type dataarray_area, datasubarray_area, datasubblock_area;
  area_type temp;
  double temp_aspect;
  double fixed_tracks_internal, fixed_tracks_external, variable_tracks;
  double data, driver_select, colmux, predecode, addresslines;
  int blocks, htree, htree_half, i, multiplier, iter_height;
  double inter_height, inter_width, total_height, total_width;

  SB = Ndwl * Ndbl / 4;
  //v4.1: using integer casting below
  //N3to8 =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd))));
  N3to8 =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd)))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  data = b0 * (RWP + ERP + EWP) * Widthtrack;
  driver_select = (2 * RWP + ERP + EWP) * 8 * B * A / b0 * Widthtrack;
  colmux = Ndbl * Nspd * (RWP + EWP + ERP) * Widthtrack;
  predecode = (RWP + ERP + EWP) * N3to8 * 8 * Widthtrack;
  addresslines = ADDRESS_BITS * (RWP + ERP + EWP) * Widthtrack;

  fixed_tracks_internal = colmux + predecode + driver_select;
  fixed_tracks_external = colmux + driver_select + addresslines;
  variable_tracks = data;

  datasubarray_area =
    datasubarray (C, B, A, Ndbl, Ndwl, Nspd, RWP, ERP, EWP, NSER,
		  techscaling_factor);
  datasubblock_area =
    datasubblock (C, B, A, Ndbl, Ndwl, Nspd, SB, b0, RWP, ERP, EWP, NSER,
		  techscaling_factor);
  //area_all_datasubarrays =
    //Ndbl * Ndwl * calculate_area (datasubarray_area,
				  //techscaling_factor) * CONVERT_TO_MMSQUARE;
  area_all_datasubarrays = Ndbl * Ndwl * datasubarray_area.height * datasubarray_area.width * CONVERT_TO_MMSQUARE;


  if (SB == 0)
    {
      if (Ndbl * Ndwl == 1)
	{
	  total_height =
	    datasubarray_area.height + fixed_tracks_external + data;
	  total_width = datasubarray_area.width + predecode;
	}
      else
	{
	  total_height =
	    2 * datasubarray_area.height + fixed_tracks_external + data;
	  total_width = datasubarray_area.width + predecode;
	}
    }
  else if (SB == 1)
    {
      total_height = datasubblock_area.height;
      total_width = datasubblock_area.width;
    }
  else if (SB == 2)
    {
      total_height = datasubblock_area.height;
      total_width =
	2 * datasubblock_area.width + fixed_tracks_external + data;
    }
  else if (SB == 4)
    {
      total_height =
	2 * datasubblock_area.height + fixed_tracks_external + data;
      total_width =
	2 * datasubblock_area.width + fixed_tracks_internal +
	variable_tracks / 2;
    }
  else if (SB == 8)
    {
      total_height =
	2 * datasubblock_area.height + fixed_tracks_internal +
	variable_tracks / 2;
      total_width =
	2 * (2 * datasubblock_area.width + variable_tracks / 4) +
	fixed_tracks_external + data;
    }

  else if (SB > 8)
    {
      blocks = SB / 4;
	  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
      //the final int value is the correct one
      //htree = (int) (logtwo_area ((double) (blocks)));
	  htree = (int) (logtwo_area ((double) (blocks)) + EPSILON);
      inter_height = datasubblock_area.height;
      inter_width = datasubblock_area.width;
      multiplier = 1;

      if (htree % 2 == 0)
	{
	  iter_height = htree / 2;
	}

      if (htree % 2 == 0)
	{
	  for (i = 0; i <= iter_height; i++)
	    {
	      if (i == iter_height)
		{
		  total_height =
		    2 * inter_height + data / blocks * multiplier +
		    fixed_tracks_external;
		  total_width =
		    2 * inter_width + data / (2 * blocks) * multiplier +
		    fixed_tracks_internal;}
	      else
		{
		  total_height =
		    2 * inter_height + data / blocks * multiplier +
		    fixed_tracks_internal;
		  total_width =
		    2 * inter_width + data / (2 * blocks) * multiplier +
		    fixed_tracks_internal;
		  inter_height = total_height;
		  inter_width = total_width;
		  multiplier = multiplier * 4;
		}
	    }
	}
      else
	{
	  htree_half = htree - 1;
	  iter_height = htree_half / 2;
	  for (i = 0; i <= iter_height; i++)
	    {
	      total_height =
		2 * inter_height + data / blocks * multiplier +
		fixed_tracks_internal;
	      total_width =
		2 * inter_width + data / (2 * blocks) * multiplier +
		fixed_tracks_internal;
	      inter_height = total_height;
	      inter_width = total_width;
	      multiplier = multiplier * 4;
	    }
	  total_width =
	    2 * inter_width + data / (2 * blocks) * multiplier +
	    fixed_tracks_external;
	}
    }

  dataarray_area.width = total_width;
  dataarray_area.height = total_height;

  temp.height = dataarray_area.width;
  temp.width = dataarray_area.height;

  temp_aspect =
    ((temp.height / temp.width) >
     1.0) ? (temp.height / temp.width) : 1.0 / (temp.height / temp.width);
  aspect_ratio_data =
    ((dataarray_area.height / dataarray_area.width) >
     1.0) ? (dataarray_area.height / dataarray_area.width) : 1.0 /
    (dataarray_area.height / dataarray_area.width);
  if (aspect_ratio_data > temp_aspect)
    {
      dataarray_area.height = temp.height;
      dataarray_area.width = temp.width;
    }

  aspect_ratio_data =
    ((dataarray_area.height / dataarray_area.width) >
     1.0) ? (dataarray_area.height / dataarray_area.width) : 1.0 /
    (dataarray_area.height / dataarray_area.width);

  return (dataarray_area);
}

area_type
tagsubarray (int baddr,int C,int B,int A,int Ndbl,int Ndwl,double Nspd,double NSubbanks,int RWP,int ERP,int EWP,int NSER,
	     double techscaling_factor)
{
  int conservative_NSER;
  area_type tagsubarray_area, tag_area, postdecode_area, colmux_area,
    precharge_area, senseamp_area, comp_area;

  conservative_NSER = 0;
  tag_area =
    subarraytag_area (baddr, C, B, A, Ndbl, Ndwl, Nspd, NSubbanks, RWP, ERP,
		      EWP, conservative_NSER, techscaling_factor);
  postdecode_area =
    decodetag_row (baddr, C, B, A, Ndbl, Nspd, Ndwl, NSubbanks, RWP, ERP,
		   EWP);
  colmux_area = colmux (Ndbl, Nspd, RWP, ERP, EWP, conservative_NSER);
  precharge_area = precharge (Ndbl, Nspd, RWP, ERP, EWP, conservative_NSER);
  senseamp_area = senseamp (Ndbl, Nspd, RWP, ERP, EWP, conservative_NSER);
  comp_area = comparatorbit (RWP, ERP, EWP);
  tagsubarray_area.height =
    tag_area.height + colmux_area.height + precharge_area.height +
    senseamp_area.height + comp_area.height;
  tagsubarray_area.width = tag_area.width + postdecode_area.width;

  return (tagsubarray_area);
}

area_type
tagsubblock (int baddr,int C,int B,int A,int Ndbl,int Ndwl,double Nspd,double NSubbanks,int SB,int RWP,int ERP,int EWP,
	     int NSER,double techscaling_factor)
{
  int N3to8, T;
  int SB_;
  //int colmuxtracks_rem, writeseltracks_rem;
  int colmuxtracks_rem;
  double tracks_h, tracks_w;
  area_type tagsubarray_area, tagsubblock_area;
  int conservative_NSER;

  conservative_NSER = 0;
  SB_ = SB;
  if (SB_ == 0)
    {
      SB_ = 1;
    }
  //v4.1: using integer casting below
  //N3to8 =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd))));
  N3to8 =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd)))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //T =
    //baddr - logtwo_area ((double) (C)) + logtwo_area ((double) (A)) + 2 -
    //logtwo_area ((double) (NSubbanks));
  T =
    (int) (baddr - logtwo_area ((double) (C)) + logtwo_area ((double) (A)) + 2 -
    logtwo_area ((double) (NSubbanks)) + EPSILON);

  colmuxtracks_rem =
    (Ndbl * Nspd >
     tracks_precharge_p) ? (Ndbl * Nspd - tracks_precharge_p) : 0;
  /*writeseltracks_rem = ((2*B*A)/(b0) > tracks_precharge_nx2) ? ((2*B*A)/(b0)-tracks_precharge_nx2) : 0; */

  tracks_h =
    Widthtrack * (N3to8 * 8 * (RWP + ERP + EWP) +
		  (RWP + EWP) * colmuxtracks_rem + Ndbl * Nspd * ERP + (RWP +
									ERP +
									EWP) *
		  T / SB_ + (ERP + RWP) * A);
  tracks_w = Widthtrack * (N3to8 * 8) * (RWP + ERP + EWP);
  tagsubarray_area =
    tagsubarray (baddr, C, B, A, Ndbl, Ndwl, Nspd, NSubbanks, RWP, ERP, EWP,
		 conservative_NSER, techscaling_factor);
  tagsubblock_area.height = 2 * tagsubarray_area.height + tracks_h;
  tagsubblock_area.width = 2 * tagsubarray_area.width + tracks_w;

  return (tagsubblock_area);
}

area_type
tagarray (int baddr,int C,int B,int A,int Ndbl,int Ndwl,double Nspd,double NSubbanks,int RWP,int ERP,int EWP,int NSER,
	  double techscaling_factor)
{
  //int SB, CSB, N3to8, T;
  int SB, N3to8, T;
  area_type tagarray_area, tagsubarray_area, tagsubblock_area;
  area_type temp;
  double temp_aspect;
  int conservative_NSER;

  double fixed_tracks_internal, fixed_tracks_external, variable_tracks;
  double tag, assoc, colmux, predecode, addresslines;
  int blocks, htree, htree_half, i, multiplier, iter_height;
  double inter_height, inter_width, total_height, total_width;

  conservative_NSER = 0;
  SB = Ndwl * Ndbl / 4;

  //v4.1: using integer casting below
  //N3to8 =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd))));
  N3to8 =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) (C / (B * A * Ndbl * Nspd)))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  //Added by Shyam to make area model sensitive to "change tag" feature
   //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  if(!force_tag) {
		//T = ADDRESS_BITS + EXTRA_TAG_BITS-(int)logtwo((double)C)+(int)logtwo((double)A)-(int)(logtwo(NSubbanks));
		T = (int) (ADDRESS_BITS + EXTRA_TAG_BITS-(int)logtwo((double)C)+(int)logtwo((double)A)-(int)(logtwo(NSubbanks)) + EPSILON);
	}
  else {
	    T = force_tag_size;
	}

  //Commented by Shyam   T =
    //baddr - logtwo_area ((double) (C)) + logtwo_area ((double) (A)) + 2 -
    //logtwo_area ((double) (NSubbanks));

  tag = T * (RWP + ERP + EWP) * Widthtrack;
  assoc = (RWP + ERP) * A * Widthtrack;
  colmux = Ndbl * Nspd * (RWP + EWP + ERP) * Widthtrack;
  predecode = (RWP + ERP + EWP) * N3to8 * 8 * Widthtrack;
  addresslines = ADDRESS_BITS * (RWP + ERP + EWP) * Widthtrack;

  tagsubarray_area =
    tagsubarray (baddr, C, B, A, Ndbl, Ndwl, Nspd, NSubbanks, RWP, ERP, EWP,
		 conservative_NSER, techscaling_factor);
  tagsubblock_area =
    tagsubblock (baddr, C, B, A, Ndbl, Ndwl, Nspd, NSubbanks, SB, RWP, ERP,
		 EWP, conservative_NSER, techscaling_factor);
  //area_all_tagsubarrays =
    //Ndbl * Ndwl * calculate_area (tagsubarray_area,
				  //techscaling_factor) * CONVERT_TO_MMSQUARE;
  area_all_tagsubarrays = Ndbl * Ndwl * tagsubarray_area.height * tagsubarray_area.width * CONVERT_TO_MMSQUARE;


  fixed_tracks_internal = colmux + predecode + assoc;
  fixed_tracks_external = colmux + assoc + addresslines;
  variable_tracks = tag;

  if (SB == 0)
    {
      if (Ndbl * Ndwl == 1)
	{
	  total_height =
	    tagsubarray_area.height + fixed_tracks_external + tag;
	  total_width = tagsubarray_area.width + predecode;
	}
      else
	{
	  total_height =
	    2 * tagsubarray_area.height + fixed_tracks_external + tag;
	  total_width = tagsubarray_area.width + predecode;
	}
    }
  if (SB == 1)
    {
      total_height = tagsubblock_area.height;
      total_width = tagsubblock_area.width;
    }
  if (SB == 2)
    {
      total_height = tagsubblock_area.height;
      total_width = 2 * tagsubblock_area.width + fixed_tracks_external + tag;
    }
  if (SB == 4)
    {
      total_height =
	2 * tagsubblock_area.height + fixed_tracks_external + tag;
      total_width =
	2 * tagsubblock_area.width + fixed_tracks_internal +
	variable_tracks / 2;
    }
  if (SB == 8)
    {
      total_height =
	2 * tagsubblock_area.height + fixed_tracks_internal +
	variable_tracks / 2;
      total_width =
	2 * (2 * tagsubblock_area.width + variable_tracks / 4) +
	fixed_tracks_external + tag;
    }
  if (SB > 8)
    {
      blocks = SB / 4;
	  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
      //the final int value is the correct one
      //htree = (int) (logtwo_area ((double) (blocks)));
	  htree = (int) (logtwo_area ((double) (blocks)) + EPSILON);
      inter_height = tagsubblock_area.height;
      inter_width = tagsubblock_area.width;
      multiplier = 1;

      if (htree % 2 == 0)
	{
	  iter_height = htree / 2;
	}

      if (htree % 2 == 0)
	{
	  for (i = 0; i <= iter_height; i++)
	    {
	      if (i == iter_height)
		{
		  total_height =
		    2 * inter_height + tag / blocks * multiplier +
		    fixed_tracks_external;
		  total_width =
		    2 * inter_width + tag / (2 * blocks) * multiplier +
		    fixed_tracks_internal;}
	      else
		{
		  total_height =
		    2 * inter_height + tag / blocks * multiplier +
		    fixed_tracks_internal;
		  total_width =
		    2 * inter_width + tag / (2 * blocks) * multiplier +
		    fixed_tracks_internal;
		  inter_height = total_height;
		  inter_width = total_width;
		  multiplier = multiplier * 4;
		}
	    }
	}
      else
	{
	  htree_half = htree - 1;
	  iter_height = htree_half / 2;
	  for (i = 0; i <= iter_height; i++)
	    {
	      total_height =
		2 * inter_height + tag / blocks * multiplier +
		fixed_tracks_internal;
	      total_width =
		2 * inter_width + tag / (2 * blocks) * multiplier +
		fixed_tracks_internal;
	      inter_height = total_height;
	      inter_width = total_width;
	      multiplier = multiplier * 4;
	    }
	  total_width =
	    2 * inter_width + tag / (2 * blocks) * multiplier +
	    fixed_tracks_external;
	}
    }

  tagarray_area.width = total_width;
  tagarray_area.height = total_height;

  temp.height = tagarray_area.width;
  temp.width = tagarray_area.height;
  temp_aspect =
    ((temp.height / temp.width) >
     1.0) ? (temp.height / temp.width) : 1.0 / (temp.height / temp.width);
  aspect_ratio_tag =
    ((tagarray_area.height / tagarray_area.width) >
     1.0) ? (tagarray_area.height / tagarray_area.width) : 1.0 /
    (tagarray_area.height / tagarray_area.width);
  if (aspect_ratio_tag > temp_aspect)
    {
      tagarray_area.height = temp.height;
      tagarray_area.width = temp.width;
    }

  aspect_ratio_tag =
    ((tagarray_area.height / tagarray_area.width) >
     1.0) ? (tagarray_area.height / tagarray_area.width) : 1.0 /
    (tagarray_area.height / tagarray_area.width);

  return (tagarray_area);
}

void
area (int baddr,int b0,int Ndbl,int Ndwl,double Nspd,int Ntbl,int Ntwl,int Ntspd,double NSubbanks,parameter_type *parameters,
      arearesult_type *result)
{
  int rows_datasubarray;
  //v4.1. Making colns_datasubarray double datatype since it is dependent on Nspd
  //which is double. Note that based on its usage below colns_datasubarray is a misnomer.
  //colns_datasubarray is actually the degree of output muxing.
  double colns_datasubarray;
  int rows_tagsubarray, colns_tagsubarray;

  rows_datasubarray =
    (parameters->cache_size /
     ((parameters->block_size) * (parameters->data_associativity) * Ndbl * Nspd));
  colns_datasubarray = (Ndbl * Nspd);
  rows_tagsubarray =
    (parameters->cache_size /
     ((parameters->block_size) * (parameters->tag_associativity) * Ntbl * Ntspd));
  colns_tagsubarray = (Ntbl * Ntspd);

  result->dataarray_area =
    dataarray (parameters->cache_size, parameters->block_size,
	       parameters->data_associativity, Ndbl, Ndwl, Nspd, b0,
	       parameters->num_readwrite_ports, parameters->num_read_ports,
	       parameters->num_write_ports,
	       parameters->num_single_ended_read_ports,
	       parameters->fudgefactor);

  result->datapredecode_area =
    predecode_area (rows_datasubarray, parameters->num_readwrite_ports,
		    parameters->num_read_ports, parameters->num_write_ports);
  result->datacolmuxpredecode_area =
    predecode_area (colns_datasubarray, parameters->num_readwrite_ports,
		    parameters->num_read_ports, parameters->num_write_ports);
  result->datacolmuxpostdecode_area =
    postdecode_area (colns_datasubarray, parameters->num_readwrite_ports,
		     parameters->num_read_ports, parameters->num_write_ports);
  result->datawritesig_area =
    muxdrvsig (parameters->data_associativity, parameters->block_size, b0);

 //if-else on pure_sram_flag added by Shyam so that tag array area not calculated in
  //pure SRAM mode
  if(!pure_sram_flag)  {

	  result->tagarray_area =
		tagarray (baddr, parameters->cache_size, parameters->block_size,
			  parameters->tag_associativity, Ntbl, Ntwl, Ntspd, NSubbanks,
			  parameters->num_readwrite_ports, parameters->num_read_ports,
			  parameters->num_write_ports,
			  parameters->num_single_ended_read_ports,
			  parameters->fudgefactor);
	  result->tagpredecode_area =
		predecode_area (rows_tagsubarray, parameters->num_readwrite_ports,
				parameters->num_read_ports, parameters->num_write_ports);
	  result->tagcolmuxpredecode_area =
		predecode_area (colns_tagsubarray, parameters->num_readwrite_ports,
				parameters->num_read_ports, parameters->num_write_ports);
	  result->tagcolmuxpostdecode_area =
		postdecode_area (colns_tagsubarray, parameters->num_readwrite_ports,
				 parameters->num_read_ports, parameters->num_write_ports);
	  result->tagoutdrvdecode_area =
		muxdriverdecode (parameters->block_size, b0,
				 parameters->num_readwrite_ports,
				 parameters->num_read_ports, parameters->num_write_ports);
	  result->tagoutdrvsig_area =
		muxdrvsig (parameters->tag_associativity, parameters->block_size, b0);
  }
  else{
		result->tagarray_area.height = 0;
		result->tagarray_area.width = 0;
		result->tagarray_area.scaled_area = 0;
		result->tagpredecode_area.height = 0;
		result->tagpredecode_area.width = 0;
		result->tagpredecode_area.scaled_area = 0;
		result->tagcolmuxpredecode_area.height = 0;
		result->tagcolmuxpredecode_area.width = 0;
		result->tagcolmuxpredecode_area.scaled_area = 0;
		result->tagcolmuxpostdecode_area.height = 0;
		result->tagcolmuxpostdecode_area.width = 0;
		result->tagcolmuxpostdecode_area.scaled_area = 0;
		result->tagoutdrvdecode_area.height = 0;
		result->tagoutdrvdecode_area.width = 0;
		result->tagoutdrvdecode_area.scaled_area = 0;
		result->tagoutdrvsig_area.height = 0;
		result->tagoutdrvsig_area.width = 0;
		result->tagoutdrvsig_area.scaled_area = 0;

  }

  /*result->totalarea =
    calculate_area (result->dataarray_area,
		    parameters->fudgefactor) +
    calculate_area (result->datapredecode_area,
		    parameters->fudgefactor) +
    calculate_area (result->datacolmuxpredecode_area,
		    parameters->fudgefactor) +
    calculate_area (result->datacolmuxpostdecode_area,
		    parameters->fudgefactor) +
    (parameters->num_readwrite_ports +
     parameters->num_write_ports) * calculate_area (result->datawritesig_area,
						    parameters->fudgefactor) +
    calculate_area (result->tagarray_area,
		    parameters->fudgefactor) +
    calculate_area (result->tagpredecode_area,
		    parameters->fudgefactor) +
    calculate_area (result->tagcolmuxpredecode_area,
		    parameters->fudgefactor) +
    calculate_area (result->tagcolmuxpostdecode_area,
		    parameters->fudgefactor) +
    calculate_area (result->tagoutdrvdecode_area,
		    parameters->fudgefactor) +
    (parameters->num_readwrite_ports +
     parameters->num_read_ports) * calculate_area (result->tagoutdrvsig_area,
						   parameters->fudgefactor);*/


  result->totalarea = result->dataarray_area.height * result->dataarray_area.width +
	  result->datapredecode_area.height * result->datapredecode_area.width  +
	  result->datacolmuxpredecode_area.height * result->datacolmuxpredecode_area.width +
	  result->datacolmuxpostdecode_area.height * result->datacolmuxpostdecode_area.width +
    (parameters->num_readwrite_ports + parameters->num_write_ports) * result->datawritesig_area.height * result->datawritesig_area.width +
	result->tagarray_area.height * result->tagarray_area.width +
	result->tagpredecode_area.height * result->tagpredecode_area.width +
	result->tagcolmuxpredecode_area.height * result->tagcolmuxpredecode_area.width +
    result->tagcolmuxpostdecode_area.height * result->tagcolmuxpostdecode_area.width +
	result->tagoutdrvdecode_area.height * result->tagoutdrvdecode_area.width +
    (parameters->num_readwrite_ports +  parameters->num_read_ports) * result->tagoutdrvsig_area.height * result->tagoutdrvsig_area.width;

}

area_type
fadecode_row (int C,int B,int Ndbl,int RWP,int ERP,int EWP)	/*returns area of post decode */
{
  int numstack;
  double decodeNORwidth, firstinv;
  area_type decinv, worddriveinv, postdecodearea;

  //v4.1: using integer casting below
  //numstack =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) ((double) C / (double) (B))));
  numstack =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) ((double) C / (double) (B)))));
  if (numstack == 0)
    numstack = 1;
  if (numstack > 6)
    numstack = 6;
  switch (numstack)
    {
    case 1:
      decodeNORwidth = WidthNOR1;
      break;
    case 2:
      decodeNORwidth = WidthNOR2;
      break;
    case 3:
      decodeNORwidth = WidthNOR3;
      break;
    case 4:
      decodeNORwidth = WidthNOR4;
      break;
    case 5:
      decodeNORwidth = WidthNOR5;
      break;
    case 6:
      decodeNORwidth = WidthNOR6;
      break;
    default:
      printf ("error:numstack=%d\n", numstack);
      printf ("Cacti does not support a series stack of %d transistors !\n",
	      numstack);
      exit (0);
      break;

    }
  decinv = inverter_area (Wdecinvp, Wdecinvn);
  worddriveinv = inverter_area (Wdecinvp, Wdecinvn);
  switch (numstack)
    {
    case 1:
      firstinv = decinv.height;
      break;
    case 2:
      firstinv = decinv.height;
      break;
    case 3:
      firstinv = decinv.height;
      break;
    case 4:
      firstinv = decNandWidth;
      break;
    case 5:
      firstinv = decNandWidth;
      break;
    case 6:
      firstinv = decNandWidth;
      break;
    default:
      printf ("error:numstack=%d\n", numstack);
      printf ("Cacti does not support a series stack of %d transistors !\n",
	      numstack);
      exit (0);
      break;

    }

  /*
  was : postdecodearea.height = BitHeight16x2;
  */
  postdecodearea.height = 2*BitHeight1x1;
  postdecodearea.width =
    (decodeNORwidth + firstinv + worddriveinv.height) * (RWP + EWP);
  return (postdecodearea);
}

area_type
fasubarray (int baddr,int C,int B,int Ndbl,int RWP,int ERP,int EWP,int NSER,double techscaling_factor)	/* returns area of subarray */
{
  area_type FAarea, fadecoderow, faramcell;
  int noof_rowsdata, noof_colnsdata;
  int Tagbits, HTagbits;
  double precharge, widthoverhead, heightoverhead;

  noof_rowsdata = (C / (B * Ndbl));
  noof_colnsdata = (8 * B);
  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //Tagbits = baddr - logtwo_area ((double) (B)) + 2;
  Tagbits = (int) (baddr - logtwo_area ((double) (B)) + 2 + EPSILON);

  //v4.1: using integer casting below
  //HTagbits = ceil ((double) (Tagbits) / 2.0);
  HTagbits = (int)(ceil ((double) (Tagbits) / 2.0));
  precharge =
    Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact + 1 / FUDGEFACTOR) +
    3 * Widthptondiff;

  if ((RWP == 1) && (ERP == 0) && (EWP == 0))
    {
      heightoverhead = 0;
      widthoverhead = 0;
    }
  else
    {
      if ((RWP == 1) && (ERP == 1) && (EWP == 0))
	{
	  widthoverhead = FAWidthIncrPer_first_r_port;
	  heightoverhead = FAHeightIncrPer_first_r_port;
	}
      else
	{
	  if ((RWP == 1) && (ERP == 0) && (EWP == 1))
	    {
	      widthoverhead = FAWidthIncrPer_first_rw_or_w_port;
	      heightoverhead = FAHeightIncrPer_first_rw_or_w_port;
	    }
	  else
	    {
	      if (RWP + EWP >= 2)
		{
		  widthoverhead =
		    FAWidthIncrPer_first_rw_or_w_port + (RWP + EWP -
							 2) *
		    FAWidthIncrPer_later_rw_or_w_port +
		    ERP * FAWidthIncrPer_later_r_port;
		  heightoverhead =
		    FAHeightIncrPer_first_rw_or_w_port + (RWP + EWP -
							  2) *
		    FAHeightIncrPer_later_rw_or_w_port +
		    ERP * FAHeightIncrPer_later_r_port;}
	      else
		{
		  if ((RWP == 0) && (EWP == 0))
		    {
		      widthoverhead =
			FAWidthIncrPer_first_r_port + (ERP -
						       1) *
			FAWidthIncrPer_later_r_port;
		      heightoverhead =
			FAHeightIncrPer_first_r_port + (ERP -
							1) *
			FAHeightIncrPer_later_r_port;;}
		  else
		    {
		      if ((RWP == 0) && (EWP == 1))
			{
			  widthoverhead = ERP * FAWidthIncrPer_later_r_port;
			  heightoverhead = ERP * FAHeightIncrPer_later_r_port;
			}
		      else
			{
			  if ((RWP == 1) && (EWP == 0))
			    {
			      widthoverhead =
				ERP * FAWidthIncrPer_later_r_port;
			      heightoverhead =
				ERP * FAHeightIncrPer_later_r_port;}
			}
		    }
		}
	    }
	}
    }

  faramcell.height =
    ceil ((double) (noof_rowsdata) / 16.0) * stitch_ramv + (CAM2x2Height_1p +
							    2 *
							    heightoverhead) *
    ceil ((double) (noof_rowsdata) / 2.0);

  /*
  was: faramcell.width=(ceil((double)(noof_colnsdata)/16.0))*(BitWidth16x2+16*(Widthtrack*2*(RWP+(ERP-NSER)+EWP-1)+Widthtrack*NSER))+2*(HTagbits*((CAM2x2Width_1p+2*widthoverhead)-Widthcontact))+(BitWidth+Widthtrack*2*(RWP+ERP+EWP-1))+(FArowNANDWidth+FArowNOR_INVWidth)*(RWP+ERP+EWP);
  */
  faramcell.width=noof_colnsdata*(BitWidth1x1+(Widthtrack*2*(RWP+(ERP-NSER)+EWP-1)+Widthtrack*NSER))+2*(HTagbits*((CAM2x2Width_1p+2*widthoverhead)-Widthcontact))+(BitWidth+Widthtrack*2*(RWP+ERP+EWP-1))+(FArowNANDWidth+FArowNOR_INVWidth)*(RWP+ERP+EWP);

  FAarea.height =
    faramcell.height + precharge * (RWP + EWP) + SenseampHeight * (RWP +
								   ERP) +
    DatainvHeight * (RWP + EWP) + FAOutdriveHeight * (RWP + ERP);
  FAarea.width = faramcell.width;

  fadecoderow = fadecode_row (C, B, Ndbl, RWP, ERP, EWP);
  FAarea.width = FAarea.width + fadecoderow.width;

  //area_all_dataramcells =
    //Ndbl * calculate_area (faramcell, techscaling_factor) * CONVERT_TO_MMSQUARE;
  //faarea_all_subarrays =
    //Ndbl * calculate_area (FAarea, techscaling_factor) * CONVERT_TO_MMSQUARE;
  area_all_dataramcells = Ndbl * faramcell.height * faramcell.width * CONVERT_TO_MMSQUARE;
  faarea_all_subarrays = Ndbl * FAarea.height * FAarea.width * CONVERT_TO_MMSQUARE;
  return (FAarea);
}

area_type
faarea (int baddr,int b0,int C,int B,int Ndbl,int RWP,int ERP,int EWP,int NSER,double techscaling_factor)
{
  area_type fasubarray_area, fa_area;
  int Tagbits, blocksel, N3to8;
  double fixed_tracks, predecode, base_height, base_width;
  area_type temp;
  double temp_aspect;

  int blocks, htree, htree_half, i, iter;
  double inter_height, inter_width, total_height, total_width;


  //v4.1: using integer casting below
  //N3to8 =
    //ceil ((1.0 / 3.0) * logtwo_area ((double) ((double) C / (double) (B))));
 N3to8 =
    (int) (ceil ((1.0 / 3.0) * logtwo_area ((double) ((double) C / (double) (B)))));
  if (N3to8 == 0)
    {
      N3to8 = 1;
    }

  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  Tagbits =  (int)(baddr - logtwo_area ((double) (B)) + 2 + EPSILON);
  fasubarray_area =
    fasubarray (baddr, C, B, Ndbl, RWP, ERP, EWP, NSER, techscaling_factor);
  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
  //the final int value is the correct one
  //blocksel = MAX (logtwo_area ((double) (B)), (8 * B) / b0);
  blocksel = MAX ((int)(logtwo_area ((double) (B)) + EPSILON), (8 * B) / b0);
  blocksel =
    (blocksel >
     tracks_outdrvfanand_p) ? (blocksel - tracks_outdrvfanand_p) : 0;

  fixed_tracks =
    Widthtrack * (1 * (RWP + EWP) + b0 * (RWP + ERP + EWP) +
		  Tagbits * (RWP + ERP + EWP) + blocksel * (RWP + ERP + EWP));
  predecode = Widthtrack * (N3to8 * 8) * (RWP + EWP);

  if (Ndbl == 1)
    {
      total_height = fasubarray_area.height + fixed_tracks;
      total_width = fasubarray_area.width + predecode;
    }
  if (Ndbl == 2)
    {
      total_height = 2 * fasubarray_area.height + fixed_tracks;
      total_width = fasubarray_area.width + predecode;
    }
  if (Ndbl == 4)
    {
      total_height = 2 * fasubarray_area.height + fixed_tracks + predecode;
      total_width = 2 * fasubarray_area.width + predecode;
    }
  if (Ndbl > 4)
    {
      blocks = Ndbl / 4;
	  //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
      //the final int value is the correct one
      //htree = (int) (logtwo_area ((double) (blocks)));
	  htree = (int) (logtwo_area ((double) (blocks)) + EPSILON);
      base_height = 2 * fasubarray_area.height + fixed_tracks + predecode;
      base_width = 2 * fasubarray_area.width + predecode;

      inter_height = base_height;
      inter_width = base_width;

      if (htree % 2 == 0)
	{
	  iter = htree / 2;
	}

      if (htree % 2 == 0)
	{
	  for (i = 1; i <= iter; i++)
	    {
	      total_height = 2 * (inter_height) + fixed_tracks + predecode;
	      inter_height = total_height;
	      total_width = 2 * (inter_width) + fixed_tracks + predecode;
	      inter_width = total_width;
	    }
	}
      else
	{
	  htree_half = htree - 1;
	  iter = htree_half / 2;
	  if (iter == 0)
	    {
	      total_height = base_height;
	      total_width = 2 * base_width + fixed_tracks + predecode;
	    }
	  else
	    {
	      for (i = 0; i <= iter; i++)
		{
		  total_height = 2 * inter_height + fixed_tracks + predecode;
		  total_width = 2 * inter_width + fixed_tracks + predecode;
		  inter_height = total_height;
		  inter_width = total_width;
		}
	      total_width = 2 * inter_width + fixed_tracks + predecode;
	    }
	}
    }

  fa_area.height = total_height;
  fa_area.width = total_width;

  temp.height = fa_area.width;
  temp.width = fa_area.height;
  temp_aspect =
    ((temp.height / temp.width) >
     1.0) ? (temp.height / temp.width) : 1.0 / (temp.height / temp.width);
  aspect_ratio_data =
    ((fa_area.height / fa_area.width) >
     1.0) ? (fa_area.height / fa_area.width) : 1.0 / (fa_area.height /
						      fa_area.width);
  if (aspect_ratio_data > temp_aspect)
    {
      fa_area.height = temp.height;
      fa_area.width = temp.width;
    }

  aspect_ratio_data =
    ((fa_area.height / fa_area.width) >
     1.0) ? (fa_area.height / fa_area.width) : 1.0 / (fa_area.height /
						      fa_area.width);

  return (fa_area);
}

void
fatotalarea (int baddr,int b0,int Ndbl,parameter_type *parameters,arearesult_type *faresult)
{
  area_type null_area;

  null_area.height = 0.0;
  null_area.width = 0.0;

  faresult->dataarray_area =
    faarea (baddr, b0, parameters->cache_size, parameters->block_size, Ndbl,
	    parameters->num_readwrite_ports, parameters->num_read_ports,
	    parameters->num_write_ports,
	    parameters->num_single_ended_read_ports, parameters->fudgefactor);
  faresult->datapredecode_area =
    predecode_area (parameters->cache_size / parameters->block_size,
		    parameters->num_readwrite_ports,
		    parameters->num_read_ports, parameters->num_write_ports);
  faresult->datacolmuxpredecode_area = null_area;
  faresult->datacolmuxpostdecode_area = null_area;
  faresult->datawritesig_area = null_area;

  faresult->tagarray_area = null_area;
  faresult->tagpredecode_area = null_area;
  faresult->tagcolmuxpredecode_area = null_area;
  faresult->tagcolmuxpostdecode_area = null_area;
  faresult->tagoutdrvdecode_area =
    muxdriverdecode (parameters->block_size, b0,
		     parameters->num_readwrite_ports,
		     parameters->num_read_ports, parameters->num_write_ports);
  faresult->tagoutdrvsig_area = null_area;
  /*faresult->totalarea =
    (calculate_area (faresult->dataarray_area, parameters->fudgefactor) +
     calculate_area (faresult->datapredecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->datacolmuxpredecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->datacolmuxpostdecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagarray_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagpredecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagcolmuxpredecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagcolmuxpostdecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagoutdrvdecode_area,
		     parameters->fudgefactor) +
     calculate_area (faresult->tagoutdrvsig_area, parameters->fudgefactor));*/

   faresult->totalarea = faresult->dataarray_area.height * faresult->dataarray_area.width +
	   faresult->datapredecode_area.height * faresult->datapredecode_area.width +
     faresult->datacolmuxpredecode_area.height * faresult->datacolmuxpredecode_area.width +
	 faresult->datacolmuxpostdecode_area.height * faresult->datacolmuxpostdecode_area.width +
     faresult->tagarray_area.height * faresult->tagarray_area.width +
	 faresult->tagpredecode_area.height * faresult->tagpredecode_area.width +
	 faresult->tagcolmuxpredecode_area.height * faresult->tagcolmuxpredecode_area.width +
	 faresult->tagcolmuxpostdecode_area.height * faresult->tagcolmuxpostdecode_area.width +
	 faresult->tagoutdrvdecode_area.height * faresult->tagoutdrvdecode_area.width +
     faresult->tagoutdrvsig_area.height * faresult->tagoutdrvsig_area.width;

}


void
area_subbanked (int baddr,int b0,int RWP,int ERP,int EWP,int Ndbl,int Ndwl,double Nspd,int Ntbl,int Ntwl,int Ntspd,
		double NSubbanks,parameter_type *parameters,area_type *result_subbanked,arearesult_type *result)
{
  arearesult_type result_area;
  area_type temp;
  double temp_aspect;
  int blocks, htree, htree_double;
  double base_height, base_width, inter_width, inter_height, total_height,
    total_width;
  int base_subbanks, inter_subbanks;
  int i, iter_height, iter_width, iter_width_double;

  area_all_dataramcells = 0.0;
  area_all_tagramcells = 0.0;
  aspect_ratio_data = 1.0;
  aspect_ratio_tag = 1.0;
  aspect_ratio_subbank = 1.0;
  aspect_ratio_total = 1.0;


  if (parameters->fully_assoc == 0)
    {
      area (baddr, b0, Ndbl, Ndwl, Nspd, Ntbl, Ntwl, Ntspd, NSubbanks,
	    parameters, &result_area);
    }
  else
    {
      fatotalarea (baddr, b0, Ndbl, parameters, &result_area);
    }

  result->dataarray_area = result_area.dataarray_area;
  result->datapredecode_area = result_area.datapredecode_area;
  result->datacolmuxpredecode_area = result_area.datacolmuxpredecode_area;
  result->datacolmuxpostdecode_area = result_area.datacolmuxpostdecode_area;
  result->datawritesig_area = result_area.datawritesig_area;
  result->tagarray_area = result_area.tagarray_area;
  result->tagpredecode_area = result_area.tagpredecode_area;
  result->tagcolmuxpredecode_area = result_area.tagcolmuxpredecode_area;
  result->tagcolmuxpostdecode_area = result_area.tagcolmuxpostdecode_area;
  result->tagoutdrvdecode_area = result_area.tagoutdrvdecode_area;
  result->tagoutdrvsig_area = result_area.tagoutdrvsig_area;
  result->totalarea = result_area.totalarea;
  result->total_dataarea = result_area.total_dataarea;
  result->total_tagarea = result_area.total_tagarea;

  if (NSubbanks == 1)
    {
      total_height = result_area.dataarray_area.height;
      total_width =
	result_area.dataarray_area.width + result_area.tagarray_area.width;
    }
  //v4.2: The height/width components were not getting multiplied by WIREPITCH. Fixing
  //that problem
  if (NSubbanks == 2)
    {
      total_height =
	result_area.dataarray_area.height + (RWP + ERP + EWP) * ADDRESS_BITS * WIREPITCH;
      total_width =
	(result_area.dataarray_area.width +
	 result_area.tagarray_area.width) * 2 + (ADDRESS_BITS +
						 BITOUT) * WIREPITCH * NSubbanks * (RWP +
									ERP +
									EWP);
    }
  if (NSubbanks == 4)
    {
      total_height =
	2 * result_area.dataarray_area.height + 2 * (RWP + ERP +
						     EWP) * ADDRESS_BITS * WIREPITCH;
      total_width =
	(result_area.dataarray_area.width +
	 result_area.tagarray_area.width) * 2 + (ADDRESS_BITS +
						 BITOUT) * WIREPITCH * NSubbanks * (RWP +
									ERP +
									EWP);

    }
  if (NSubbanks == 8)
    {
      total_height =
	(result_area.dataarray_area.width +
	 result_area.tagarray_area.width) * 2 + (ADDRESS_BITS +
						 BITOUT) * WIREPITCH * NSubbanks * (RWP +
									ERP +
									EWP) *
	0.5;
      total_width =
	2 * (2 * result_area.dataarray_area.height +
	     2 * (RWP + ERP + EWP) * ADDRESS_BITS * WIREPITCH) + (ADDRESS_BITS +
						      BITOUT) * WIREPITCH * NSubbanks *
	(RWP + ERP + EWP);
    }

  if (NSubbanks > 8)
    {
	   //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
       //the final int value is the correct one
      //blocks = NSubbanks / 16 ;
	  blocks = (int) (NSubbanks / 16 + EPSILON) ;
      //htree = (int) (logtwo_area ((double) (blocks)));
	  htree = (int) (logtwo_area ((double) (blocks)) + EPSILON);
      base_height =
	2 *
	((result_area.
	  dataarray_area.width + result_area.tagarray_area.width) * 2 +
	 (ADDRESS_BITS + BITOUT) * WIREPITCH * 16 * (RWP + ERP + EWP) * 0.25) +
	(ADDRESS_BITS + BITOUT) * WIREPITCH * 16 * (RWP + ERP + EWP) * 0.5;
      base_width =
	2 * (2 * result_area.dataarray_area.height +
	     2 * (RWP + ERP + EWP) * ADDRESS_BITS) * WIREPITCH + (ADDRESS_BITS +
						      BITOUT) * WIREPITCH * 16 * (RWP +
								      ERP +
								      EWP) *
	0.25;
      base_subbanks = 16;
      if (htree % 2 == 0)
	{
	  iter_height = htree / 2;
	}
      else
	{
	  iter_height = (htree - 1) / 2;
	}

      inter_height = base_height;
      inter_subbanks = base_subbanks;

      if (iter_height == 0)
	{
	  total_height = base_height;
	}
      else
	{
	  for (i = 1; i <= iter_height; i++)
	    {
	      total_height =
		2 * (inter_height) + (ADDRESS_BITS +
				      BITOUT) * WIREPITCH * 4 * inter_subbanks * (RWP +
								      ERP +
								      EWP) *
		0.5;
	      inter_height = total_height;
	      inter_subbanks = inter_subbanks * 4;
	    }
	}

      inter_width = base_width;
      inter_subbanks = base_subbanks;
      iter_width = 10;

      if (htree % 2 == 0)
	{
	  iter_width = htree / 2;
	}

      if (iter_width == 0)
	{
	  total_width = base_width;
	}
      else
	{
	  if (htree % 2 == 0)
	    {
	      for (i = 1; i <= iter_width; i++)
		{
		  total_width =
		    2 * (inter_width) + (ADDRESS_BITS +
					 BITOUT) * WIREPITCH * inter_subbanks * (RWP +
								     ERP +
								     EWP);
		  inter_width = total_height;
		  inter_subbanks = inter_subbanks * 4;
		}
	    }
	  else
	    {
	      htree_double = htree + 1;
	      iter_width_double = htree_double / 2;
	      for (i = 1; i <= iter_width_double; i++)
		{
		  total_width =
		    2 * (inter_width) + (ADDRESS_BITS +
					 BITOUT) * WIREPITCH * inter_subbanks * (RWP +
								     ERP +
								     EWP);
		  inter_width = total_height;
		  inter_subbanks = inter_subbanks * 4;
		}
	      total_width +=
		(ADDRESS_BITS + BITOUT) * WIREPITCH * (RWP + ERP + EWP) * NSubbanks / 2;
	    }
	}
    }

  result_subbanked->height = total_height;
  result_subbanked->width = total_width;

  temp.width = result_subbanked->height;
  temp.height = result_subbanked->width;

  temp_aspect =
    ((temp.height / temp.width) >
     1.0) ? (temp.height / temp.width) : 1.0 / (temp.height / temp.width);

  aspect_ratio_total = (result_subbanked->height / result_subbanked->width);

  aspect_ratio_total =
    (aspect_ratio_total >
     1.0) ? (aspect_ratio_total) : 1.0 / (aspect_ratio_total);


  if (aspect_ratio_total > temp_aspect)
    {
      result_subbanked->height = temp.height;
      result_subbanked->width = temp.width;
    }

  aspect_ratio_subbank =
    (result_area.dataarray_area.height /
     (result_area.dataarray_area.width + result_area.tagarray_area.width));
  aspect_ratio_subbank =
    (aspect_ratio_subbank >
     1.0) ? (aspect_ratio_subbank) : 1.0 / (aspect_ratio_subbank);
  aspect_ratio_total = (result_subbanked->height / result_subbanked->width);
  aspect_ratio_total =
    (aspect_ratio_total >
     1.0) ? (aspect_ratio_total) : 1.0 / (aspect_ratio_total);

}


int
data_organizational_parameters_valid (int B,int A,int C,int Ndwl,int Ndbl,double Nspd,char assoc,double NSubbanks)
{
	int temp = 0;
	double before = 0.0;
	int tagbits = 0;
	int data_rows = 0, data_columns = 0, tag_rows = 0, tag_columns = 0;
  /* don't want more than 8 subarrays for each of data/tag */

  if (assoc == 0) {
		if (Ndwl * Ndbl > MAXSUBARRAYS) {return (FALSE);}
		/* add more constraints here as necessary */
		//v4.1: Number of rows per subarray is (C / (B * A * Ndbl * Nspd),
		//not (C / (8 * B * A * Ndbl * Nspd)
		//if (C / (8 * B * A * Ndbl * Nspd) <= 0) {return (FALSE);}
		if (C / (B * A * Ndbl * Nspd) <= 0) {return (FALSE);}
		if ((8 * B * A * Nspd / Ndwl) <= 0) {return (FALSE);}
		/*dt: Don't want ridicolously small arrays*/
		/*dt: data side: number of rows should be greater than 8 */

		//v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
		//the final int value is the correct one
		//data_rows = C/(B*A*Ndbl*Nspd) ;
		data_rows = (int) (C/(B*A*Ndbl*Nspd) + EPSILON);
		if(8 > data_rows) {return (FALSE);}
		//dt: data side: number of rows should be less than 4k
		if(2048 < data_rows) {return (FALSE);}
		//dt: data side: number of columns should be greater than 16
		//v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
        //the final int value is the correct one
		//data_columns = 8*B*A*Nspd/Ndwl;
		data_columns = (int) (8*B*A*Nspd/Ndwl + EPSILON);
		if(16 > data_columns) {return (FALSE);}
		//dt: data side: number of columns should be less than 1k
		if(512 < data_columns) {return (FALSE);}

		//Added by Shyam to specify a minimum 8KB mat (subarray) size in the data array
		//if((C <= 8192)&&((Ndwl > 1)||(Ndbl > 1))) return (FALSE);
		//End of Shyam's minimum mat size change

  }
  else {
		if (C / (2 * B * Ndbl) <= 0) {return (FALSE);}
		if (Ndbl > MAXDATAN) {return (FALSE);}
  }

  return (TRUE);
}

int
tag_organizational_parameters_valid (int B,int A,int C,int Ntwl,int Ntbl,int Ntspd,char assoc,double NSubbanks)
{
	int temp = 0;
	double before = 0.0;
	int tagbits = 0;
	int data_rows = 0, data_columns = 0, tag_rows = 0, tag_columns = 0;
  /* don't want more than 8 subarrays for each of data/tag */

  if (assoc == 0) {
		if (Ntwl * Ntbl > MAXSUBARRAYS) {return (FALSE);}
		/* add more constraints here as necessary */
		//v4.1: Number of rows per subarray is (C / (B * A * Ntbl * Ntspd)
		//not (C / (8 * B * A * Ntbl * Ntspd)
		//if (C / (8 * B * A * Ntbl * Ntspd) <= 0) {return (FALSE);}
		if (C / (B * A * Ntbl * Ntspd) <= 0) {return (FALSE);}
		if ((8 * B * A * Ntspd / Ntwl) <= 0) {return (FALSE);}
		/*dt: Don't want ridicolously small arrays*/
		/*dt: tag side: number of rows should be greater than 8 */

		tag_rows = C/(B*A*Ntbl*Ntspd);
		//Shyam for vX.X: tag_rows = number of sets is max when Ntbl=Ntspd=1. Checking that
		//tagrows is at least 1 instead of 8. FIX THIS LATER
		if(8 > tag_rows) {return (FALSE);}
		//dt: tag side: number of rows should be less than 1k
		if(512 < tag_rows) {return (FALSE);}
		// dt: tag side: number of column should be greater than 8
		tagbits = ADDRESS_BITS+EXTRA_TAG_BITS;
		 //v4.1: Fixing double->int type conversion problems. EPSILON is added below to make sure
        //the final int value is the correct one
		//before = logtwo((double)C);
		before = (int) (logtwo((double)C) + EPSILON);
		temp = (int)(before);
		tagbits -= temp;
		//tagbits += (int)logtwo((double)A);
		//tagbits -= (int)(logtwo(NSubbanks));
		tagbits += (int) (logtwo((double)A) + EPSILON);
		tagbits -= (int)((logtwo(NSubbanks)) + EPSILON);
		tag_columns = tagbits * A * Ntspd/Ntwl;
		if(8 > tag_columns) {return (FALSE);}
		// dt: tag side: number of column should be less than 4k
		if(512 < tag_columns) {return (FALSE);}


  }
  else {
		if (Ntbl > MAXTAGN) {return (FALSE);}

  }

  return (TRUE);
}
