#ifdef __cplusplus
extern "C" {
#endif
/* fake SIM_port.h, for testing module functions without the module itself */

/* this file lists the parameters of Alpha 21364 router */
#ifndef _SIM_PORT_H
#define _SIM_PORT_H

#ifndef POWER_TEST
#define POWER_TEST
#endif

#define PARM_POWER_STATS	1

#include "SIM_power.h"
#include "SIM_power_array.h"
#include "SIM_power_router.h"
#include "SIM_power_misc.h"

/* RF module parameters */
#define PARM_read_port	1
#define PARM_write_port	1
#define PARM_n_regs	64
#define PARM_reg_width	32

#define PARM_ndwl	1
#define PARM_ndbl	1
#define PARM_nspd	1

//#define PARM_wordline_model	CACHE_RW_WORDLINE
//#define PARM_bitline_model	RW_BITLINE
//#define PARM_mem_model		NORMAL_MEM
//#define PARM_row_dec_model	SIM_NO_MODEL
//#define PARM_row_dec_pre_model	SIM_NO_MODEL
//#define PARM_col_dec_model	SIM_NO_MODEL
//#define PARM_col_dec_pre_model	SIM_NO_MODEL
//#define PARM_mux_model		SIM_NO_MODEL
//#define PARM_outdrv_model	SIM_NO_MODEL

/* these 3 should be changed together */
//#define PARM_data_end		2
//#define PARM_amp_model		GENERIC_AMP
//#define PARM_bitline_pre_model	EQU_BITLINE
//#define PARM_data_end		1
//#define PARM_amp_model		SIM_NO_MODEL
//#define PARM_bitline_pre_model	SINGLE_OTHER


/* router module parameters */
/* general parameters */
#define PARM_in_port		4
#define PARM_cache_in_port	1
#define PARM_mc_in_port		2
#define PARM_io_in_port		1
#define PARM_out_port		4
#define PARM_cache_out_port	2
#define PARM_mc_out_port	0
#define PARM_io_out_port	1
/* 4B flit */
#define PARM_flit_width		32

/* virtual channel parameters */
#define PARM_v_channel		3
#define PARM_v_class		19
#define PARM_cache_class	5
#define PARM_mc_class		3
#define PARM_io_class		5
/* ?? */
#define PARM_in_share_buf	1
#define PARM_out_share_buf	1
#define PARM_in_share_switch	0
#define PARM_out_share_switch	1

/* crossbar parameters */
#define PARM_crossbar_model	MATRIX_CROSSBAR
#define PARM_crsbar_degree	4
#define PARM_connect_type	TRISTATE_GATE
#define PARM_trans_type		NP_GATE
/* measured from Alpha 21364 floorplan */
#define PARM_crossbar_in_len	610
#define PARM_crossbar_out_len	2440

/* input buffer parameters */
#define PARM_in_buf		1
#define PARM_in_buf_set		319
#define PARM_in_buf_rport	2

#define PARM_cache_in_buf	1
#define PARM_cache_in_buf_set	250
#define PARM_cache_in_buf_rport	1

#define PARM_mc_in_buf		1
#define PARM_mc_in_buf_set	127
#define PARM_mc_in_buf_rport	2

#define PARM_io_in_buf		1
#define PARM_io_in_buf_set	190
#define PARM_io_in_buf_rport	1

/* output buffer parameters */
#define PARM_out_buf		0
#define PARM_out_buf_set	16
#define PARM_out_buf_wport	1

/* central buffer parameters */
#define PARM_central_buf	0
#define PARM_cbuf_set		1024
#define PARM_cbuf_rport		2
#define PARM_cbuf_wport		2
#define PARM_cbuf_width		8
#define PARM_pipe_depth		4

/* array parameters shared by various buffers */
#define PARM_wordline_model	CACHE_RW_WORDLINE
#define PARM_bitline_model	RW_BITLINE
#define PARM_mem_model		NORMAL_MEM
#define PARM_row_dec_model	GENERIC_DEC
#define PARM_row_dec_pre_model	SINGLE_OTHER
#define PARM_col_dec_model	SIM_NO_MODEL
#define PARM_col_dec_pre_model	SIM_NO_MODEL
#define PARM_mux_model		SIM_NO_MODEL
#define PARM_outdrv_model	REG_OUTDRV

/* these 3 should be changed together */
/* use double-ended bitline because the array is too large */
#define PARM_data_end		2
#define PARM_amp_model		GENERIC_AMP
#define PARM_bitline_pre_model	EQU_BITLINE
//#define PARM_data_end		1
//#define PARM_amp_model		SIM_NO_MODEL
//#define PARM_bitline_pre_model	SINGLE_OTHER

/* arbiter parameters */
#define PARM_in_arb_model	MATRIX_ARBITER
#define PARM_in_arb_ff_model	NEG_DFF
#define PARM_out_arb_model	MATRIX_ARBITER
#define PARM_out_arb_ff_model	NEG_DFF

#endif	/* _SIM_PORT_H */
#ifdef __cplusplus
}
#endif
