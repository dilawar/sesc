#ifndef CACTI_INTERFACE_H
#define CACTI_INTERFACE_H
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


/*dt: currently we assume that everything is counted in bytes*/
#define CHUNKSIZE 8


typedef struct {
	double dynamic;
	double leakage;
} powerComponents;

typedef struct {
	powerComponents readOp;
	powerComponents writeOp;
} powerDef;

/* Used to pass values around the program */

struct cache_params_t
{
  unsigned int nsets;
  unsigned int assoc;
  unsigned int dbits;
  unsigned int tbits;

  unsigned int nbanks;
  unsigned int rport;
  unsigned int wport;
  unsigned int rwport;
  unsigned int serport; // single-ended bitline read ports

  unsigned int obits;
  unsigned int abits;

  double dweight;
  double pweight;
  double aweight;
};

struct subarray_params_t
{
  unsigned int Ndwl;
  unsigned int Ndbl;
  unsigned int Nspd;
  unsigned int Ntwl;
  unsigned int Ntbl;
  unsigned int Ntspd;
  unsigned int muxover;
};


struct tech_params_t
{
  double tech_size;
  double crossover;
  double standby;
  double VddPow;
  double scaling_factor;
};


typedef struct {
	double height;
	double width;
	double scaled_area;
}area_type;

typedef struct {
        area_type dataarray_area,datapredecode_area;
        area_type datacolmuxpredecode_area,datacolmuxpostdecode_area;
		area_type datawritesig_area;
        area_type tagarray_area,tagpredecode_area;
        area_type tagcolmuxpredecode_area,tagcolmuxpostdecode_area;
        area_type tagoutdrvdecode_area;
        area_type tagoutdrvsig_area;
        double totalarea, subbankarea;
		double total_dataarea;
        double total_tagarea;
		double max_efficiency, efficiency;
		double max_aspect_ratio_total, aspect_ratio_total;
		double perc_data, perc_tag, perc_cont, sub_eff, total_eff;
} arearesult_type;

typedef struct {
  int cache_size;
  int number_of_sets;
  //int associativity;
  int tag_associativity, data_associativity;
  int block_size;
  int num_write_ports;
  int num_readwrite_ports;
  int num_read_ports;
  int num_single_ended_read_ports;
  int NSubbanks;
  char fully_assoc;
  double fudgefactor;
  double tech_size;
  double VddPow;
  int sequential_access;
  int fast_access;
  int force_tag;
  int tag_size;
  int nr_bits_out;
  int pure_sram;
//#ifdef XCACTI
//  int latchsa;
//  int ignore_tag;
//#endif
} parameter_type;

typedef struct {
	int subbanks;
   double access_time,cycle_time;
  double senseext_scale;
  powerDef total_power;
   int best_Ndwl,best_Ndbl;
  double max_power, max_access_time;
   double best_Nspd;
   int best_Ntwl,best_Ntbl;
   int best_Ntspd;
   int best_muxover;
   powerDef total_routing_power;
   powerDef total_power_without_routing, total_power_allbanks;
   double subbank_address_routing_delay;
   powerDef subbank_address_routing_power;
   double decoder_delay_data,decoder_delay_tag;
   powerDef decoder_power_data,decoder_power_tag;
   double dec_data_driver,dec_data_3to8,dec_data_inv;
   double dec_tag_driver,dec_tag_3to8,dec_tag_inv;
   double wordline_delay_data,wordline_delay_tag;
   powerDef wordline_power_data,wordline_power_tag;
   double bitline_delay_data,bitline_delay_tag;
   powerDef bitline_power_data,bitline_power_tag;
  double sense_amp_delay_data,sense_amp_delay_tag;
  powerDef sense_amp_power_data,sense_amp_power_tag;
  double total_out_driver_delay_data;
  powerDef total_out_driver_power_data;
   double compare_part_delay;
   double drive_mux_delay;
   double selb_delay;
   powerDef compare_part_power, drive_mux_power, selb_power;
   double data_output_delay;
   powerDef data_output_power;
   double drive_valid_delay;
   powerDef drive_valid_power;
   double precharge_delay;
  int data_nor_inputs;
  int tag_nor_inputs;
} result_type;

typedef struct{
	result_type result;
	arearesult_type area;
	area_type arearesult_subbanked;
	parameter_type params;
} total_result_type;


typedef struct{
  // Layout per bank in ratio of area (there are NSubbanks)
  //
  //    +------------------------------+
  //    |         bank ctrl            |
  //    +------------------------------+
  //    |  tag   | de |  data          |
  //    | array  | co |  array         |
  //    |        | de |                |
  //    +------------------------------+
  //    | tag_ctrl | data_ctrl         |
  //    +------------------------------+

  double bank_ctrl_a;
  double decode_a;
  double tag_array_a;
  double tag_ctrl_a;
  double data_array_a;
  double data_ctrl_a;
  double total_a;

  double bank_ctrl_e;
  double decode_e;
  double tag_array_e;
  double tag_ctrl_e;
  double data_array_e;
  double data_ctrl_e;

  int NSubbanks;
  int assoc;
} xcacti_flp;


total_result_type cacti_interface(
		int cache_size,
		int line_size,
		int associativity,
		int rw_ports,
		int excl_read_ports,
		int excl_write_ports,
		int single_ended_read_ports,
		int banks,
		double tech_node,
		int output_width,
		int specific_tag,
		int tag_width,
		int access_mode,
		int pure_sram);

void xcacti_power_flp(const result_type *result,
                      const arearesult_type *arearesult,
                      const area_type *arearesult_subbanked,
                      const parameter_type *parameters,
                      xcacti_flp *xflp);

typedef struct{
	double delay;
	powerDef power;
}output_tuples;

typedef struct{
	output_tuples
			decoder,
			wordline,
			bitline,
			senseamp;
	double senseamp_outputrisetime;
}cached_tag_entry;

#endif
