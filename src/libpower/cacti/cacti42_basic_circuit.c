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


#include "cacti42_def.h"
#include "stdio.h"
#include "math.h"

int powers (int base, int n)
{
  int i, p;

  p = 1;
  for (i = 1; i <= n; ++i)
    p *= base;
  return p;
}

/*----------------------------------------------------------------------*/

double logtwo (double x)
{
  if (x <= 0)
    printf ("%e\n", x);
  return ((double) (log (x) / log (2.0)));
}

/*----------------------------------------------------------------------*/

double gatecap (double width,double  wirelength)	/* returns gate capacitance in Farads */
     /* width: gate width in um (length is Leff) */
     /* wirelength: poly wire length going to gate in lambda */
{
  return (width * Leff * Cgate + wirelength * Cpolywire * Leff);
}

double gatecappass (double width,double  wirelength)	/* returns gate capacitance in Farads */
     /* width: gate width in um (length is Leff) */
     /* wirelength: poly wire length going to gate in lambda */
{
  return (width * Leff * Cgatepass + wirelength * Cpolywire * Leff);
}


/*----------------------------------------------------------------------*/

/* Routine for calculating drain capacitances.  The draincap routine
 * folds transistors larger than 10um */

double draincap (double width,int nchannel,int stack)	/* returns drain cap in Farads */
	 /* width: in um */
     /* nchannel: whether n or p-channel (boolean) */
     /* stack: number of transistors in series that are on */
{
  double Cdiffside, Cdiffarea, Coverlap, cap;

  Cdiffside = (nchannel) ? Cndiffside : Cpdiffside;
  Cdiffarea = (nchannel) ? Cndiffarea : Cpdiffarea;
  Coverlap = (nchannel) ? (Cndiffovlp + Cnoxideovlp) :
                          (Cpdiffovlp + Cpoxideovlp);
  /* calculate directly-connected (non-stacked) capacitance */
  /* then add in capacitance due to stacking */
  if(stack > 1) {
	if (width >= 10/FUDGEFACTOR) {
		cap = 3.0 * Leff * width / 2.0 * Cdiffarea + 6.0 * Leff * Cdiffside +
		width * Coverlap;
		cap += (double) (stack - 1) * (Leff * width * Cdiffarea +
						4.0 * Leff * Cdiffside +
						2.0 * width * Coverlap);
	}
	else {
		cap =
		3.0 * Leff * width * Cdiffarea + (6.0 * Leff + width) * Cdiffside +
		width * Coverlap;
		cap +=
		(double) (stack - 1) * (Leff * width * Cdiffarea +
					2.0 * Leff * Cdiffside +
					2.0 * width * Coverlap);
	}
  }
  else {
	  if (width >= 10/FUDGEFACTOR) {
		cap = 3.0 * Leff * width / 2.0 * Cdiffarea + 6.0 * Leff * Cdiffside +
		width * Coverlap;
	}
	else {
		cap = 3.0 * Leff * width * Cdiffarea + (6.0 * Leff + width) * Cdiffside +
		width * Coverlap;
	}
  }
  return (cap);
}

/*----------------------------------------------------------------------*/

/* The following routines estimate the effective resistance of an
   on transistor as described in the tech report.  The first routine
   gives the "switching" resistance, and the second gives the
   "full-on" resistance */

double transresswitch (double width,int nchannel,int stack)	/* returns resistance in ohms */
     /* width: in um */
     /* nchannel: whether n or p-channel (boolean) */
     /* stack: number of transistors in series */
{
  double restrans;
  restrans = (nchannel) ? (Rnchannelstatic) : (Rpchannelstatic);
  /* calculate resistance of stack - assume all but switching trans
     have 0.8X the resistance since they are on throughout switching */
  return ((1.0 + ((stack - 1.0) * 0.8)) * restrans / width);
}

/*----------------------------------------------------------------------*/

double transreson (double width,int nchannel,int stack)	/* returns resistance in ohms */
     /* width: in um */
     /* nchannel: whether n or p-channel (boolean) */
     /* stack: number of transistors in series */
{
  double restrans;
  restrans = (nchannel) ? Rnchannelon : Rpchannelon;

  /* calculate resistance of stack.  Unlike transres, we don't
     multiply the stacked transistors by 0.8 */
  return (stack * restrans / width);

}

/*----------------------------------------------------------------------*/

/* This routine operates in reverse: given a resistance, it finds
 * the transistor width that would have this R.  It is used in the
 * data wordline to estimate the wordline driver size. */

double restowidth (double res,int nchannel)	/* returns width in um */
     /* res: resistance in ohms */
     /* nchannel: whether N-channel or P-channel */
{
  double restrans;

  restrans = (nchannel) ? Rnchannelon : Rpchannelon;

  return (restrans / res);

}

/*----------------------------------------------------------------------*/

double horowitz (double inputramptime,double  tf,double  vs1,double  vs2,int rise)
	/* inputramptime: input rise time */
    /* tf: time constant of gate */
    /* vs1, vs2: threshold voltages */
    /* rise: whether INPUT rise or fall (boolean) */
{
  double a, b, td;

  a = inputramptime / tf;
  if (rise == RISE)
    {
      b = 0.5;
      td = tf * sqrt (log (vs1) * log (vs1) + 2 * a * b * (1.0 - vs1)) +
	tf * (log (vs1) - log (vs2));
    }
  else
    {
      b = 0.4;
      td = tf * sqrt (log (1.0 - vs1) * log (1.0 - vs1) + 2 * a * b * (vs1)) +
	tf * (log (1.0 - vs1) - log (1.0 - vs2));
    }
  return (td);
}
