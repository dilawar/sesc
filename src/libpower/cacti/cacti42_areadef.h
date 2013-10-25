#ifndef AREADEF_H
#define AREADEF_H
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

#include "cacti_interface.h"

double area_all_datasubarrays;
double area_all_tagsubarrays;
double area_all_dataramcells;
double area_all_tagramcells;
double faarea_all_subarrays ;

double aspect_ratio_data;
double aspect_ratio_tag;
double aspect_ratio_subbank;
double aspect_ratio_total;

/*
was: #define Widthptondiff 4.0
now: is 4* feature size, taken from the Intel 65nm process paper
*/

//v4.1: Making all constants static variables. Initially these variables are based
//off 0.8 micron process values; later on in init_tech_params function of leakage.c
//they are scaled to input tech node parameters

//#define Widthptondiff 3.2
double Widthptondiff;
/*
was: #define Widthtrack    3.2
now: 3.2*FEATURESIZE, i.e. 3.2*0.8
*/
//#define Widthtrack    (3.2*0.8)
double Widthtrack;
//#define Widthcontact  1.6
double Widthcontact;
//#define Wpoly         0.8
double Wpoly;
//#define ptocontact    0.4
double ptocontact;
//#define stitch_ramv   6.0
double stitch_ramv;
//#define BitHeight16x2 33.6
//#define BitHeight1x1 (2*7.746*0.8) /* see below */
double BitHeight1x1;
//#define stitch_ramh   12.0
double stitch_ramh;
//#define BitWidth16x2  192.8
//#define BitWidth1x1	  (7.746*0.8)
double BitWidth1x1;
/* dt: Assume that each 6-T SRAM cell is 120F^2 and has an aspect ratio of 1(width) to 2(height), than the width is 2*sqrt(60)*F */
//#define WidthNOR1     11.6
double WidthNOR1;
//#define WidthNOR2     13.6
double WidthNOR2;
//#define WidthNOR3     20.8
double WidthNOR3;
//#define WidthNOR4     28.8
double WidthNOR4;
//#define WidthNOR5     34.4
double WidthNOR5;
//#define WidthNOR6     41.6
double WidthNOR6;
//#define Predec_height1    140.8
double Predec_height1;
//#define Predec_width1     270.4
double Predec_width1;
//#define Predec_height2    140.8
double Predec_height2;
//#define Predec_width2     539.2
double Predec_width2;
//#define Predec_height3    281.6
double Predec_height3;
//#define Predec_width3     584.0
double Predec_width3;
//#define Predec_height4    281.6
double Predec_height4;
//#define Predec_width4     628.8
double Predec_width4;
//#define Predec_height5    422.4
double Predec_height5;
//#define Predec_width5     673.6
double Predec_width5;
//#define Predec_height6    422.4
double Predec_height6;
//#define Predec_width6     718.4
double Predec_width6;
//#define Wwrite		  1.2
double Wwrite;
//#define SenseampHeight    152.0
double SenseampHeight;
//#define OutdriveHeight	  200.0
double OutdriveHeight;
//#define FAOutdriveHeight  229.2
double FAOutdriveHeight;
//#define FArowWidth	  382.8
double FArowWidth;
//#define CAM2x2Height_1p	  48.8
double CAM2x2Height_1p;
//#define CAM2x2Width_1p	  44.8
double CAM2x2Width_1p;
//#define CAM2x2Height_2p   80.8
double CAM2x2Height_2p;
//#define CAM2x2Width_2p    76.8
double CAM2x2Width_2p;
//#define DatainvHeight     25.6
double DatainvHeight;
//#define Wbitdropv 	  30.0
double Wbitdropv;
//#define decNandWidth      34.4
double decNandWidth;
//#define FArowNANDWidth    71.2
double FArowNANDWidth;
//#define FArowNOR_INVWidth 28.0
double FArowNOR_INVWidth;

//#define FAHeightIncrPer_first_rw_or_w_port 16.0
double FAHeightIncrPer_first_rw_or_w_port;
//#define FAHeightIncrPer_later_rw_or_w_port 16.0
double FAHeightIncrPer_later_rw_or_w_port;
//#define FAHeightIncrPer_first_r_port       12.0
double FAHeightIncrPer_first_r_port;
//#define FAHeightIncrPer_later_r_port       12.0
double FAHeightIncrPer_later_r_port;
//#define FAWidthIncrPer_first_rw_or_w_port  16.0
double FAWidthIncrPer_first_rw_or_w_port;
//#define FAWidthIncrPer_later_rw_or_w_port  9.6
double FAWidthIncrPer_later_rw_or_w_port;
//#define FAWidthIncrPer_first_r_port        12.0
double FAWidthIncrPer_first_r_port;
//#define FAWidthIncrPer_later_r_port        9.6
double FAWidthIncrPer_later_r_port;

#define tracks_precharge_p    12
#define tracks_precharge_nx2   5
#define tracks_outdrvselinv_p  3
#define tracks_outdrvfanand_p  6

#define CONVERT_TO_MMSQUARE 1.0/1000000.0
int
data_organizational_parameters_valid(int B,int A,int C,int Ndwl,int Ndbl,double Nspd,char assoc,double NSubbanks);
int
tag_organizational_parameters_valid(int B,int A,int C,int Ntwl,int Ntbl,int Ntspd,char assoc,double NSubbanks);
void
area_subbanked (int baddr,int b0,int RWP,int ERP,int EWP,int Ndbl,int Ndwl,double Nspd,int Ntbl,int Ntwl,int Ntspd,
		double NSubbanks,parameter_type *parameters,area_type *result_subbanked,arearesult_type *result);

#endif
