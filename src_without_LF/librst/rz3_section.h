/*
* ========== Copyright Header Begin ==========================================
* 
* OpenSPARC T1 Processor File: rz3_section.h
* Copyright (c) 2006 Sun Microsystems, Inc.  All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
* 
* The above named program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License version 2 as published by the Free Software Foundation.
* 
* The above named program is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
* 
* ========== Copyright Header End ============================================
*/

#ifndef _rz3_section_h_
#define _rz3_section_h_

#include <stdio.h>
#include <strings.h>

#include "rz3utils.h"

/* we use a 16-character magic string to identify an rz3 compressed trace
 * starting with "RZ3 " and ending with a $ and the null-terminator (\0).
 * The remainder (16-4-2) is a 10-digit random integer
 */

/* rz3 section header magic: "RZ3 SHDR<int6>$" */
#define rz3_shdr_magic "3ZR SHDR297302$"

struct rz3_section_header {
  char magic[16];

  // other meta data about the section
  int32_t nrecords;

  uint64_t CompressedBufferSize;

  int32_t rz3_bitarray_counts[rstzip3::bitarray_count];

  void clear();

  bool write(gzFile gzf);

  bool read(gzFile gzf);

  bool write(FILE * fp);

  bool read(FILE * fp);

  bool sanity_check();
}; // struct rz3_section_header


/* the order in which the compressed data arrays are stored
 * NOTE: this struct is NOT read or written to/from an rz3 compressed file
 * it is included here to indicate the *order* in which the data arrays are stored
 */
struct rz3_section_data {

  // rz3_rst_array * raw_records_array; // records that we do not try to compress

  rz3_bitarray * bitarrays[rstzip3::bitarray_count];

  // we read(write) section header to determine (based on) section data
  // note: some bitarray sizes changed in v3.20
  // the pre320 flag is used to determine version-specific bitarray sizes
  rz3_section_data(rz3_section_header * arg_shdr, bool pre320);

  ~rz3_section_data();

  void clear();

  void update_counts();

  void print(); // print stats

  void print_totals();

  // bool write(FILE * fp); // returns false on error
  bool write(gzFile gzf); // returns false on error

  bool read(gzFile gzf);

  bool write(FILE * fp);

  bool read(FILE * fp);

  rz3_section_header * shdr;

  uint64_t total_nrecords;
  uint64_t total_CompressedBufferSize;
  uint64_t total_rz3_bitarray_counts[rstzip3::bitarray_count];
  uint64_t total_rz3_bitarray_sums[rstzip3::bitarray_count];
}; // struct rz3_section_data



// this struct contains temp state variables used to compress/decompress records
struct rz3_percpu_data {
  int32_t cpuid;
  
  uint64_t pred_pc;
  uint64_t pred_npc;

  uint16_t pred_icontext;
  uint16_t pred_dcontext;

  uint32_t pred_amask;
  uint32_t pred_an;
  uint32_t pred_hpriv;
  uint32_t pred_pr;

  bool call_delay_slot;

  int32_t pending_pavadiff_idx; // -1 implies invalid
  bool pending_pavadiff_pc_pa_va_pred;
  bool pending_pavadiff_ea_pa_va_pred;

  uint64_t prev_pc; // for regval predictor lookup

  uint64_t rfs_prev_npc; // for rfs branch trace records

  rz3_valuecache * valuecache;

  rz3iu_icache * icache;

  rz3iu_brpred * bp;

  rz3_ras * ras;

  rz3_table<uint64_t> * jmpl_table;

  rz3_table<uint64_t> * itlb;
  rz3_table<uint64_t> * dtlb;

  rz3_table<uint64_t> * rfs_pc_pred_table;

  uint8_t * regval_regtype_tbl[2];
  uint8_t * regval_regid_tbl[2];


  uint32_t last_instr;

  uint64_t * regs;
  uint8_t ccr;

  enum consts { 
    rz3_tdata_jmpl_table_size = 2<<10, 
    rz3_tdata_regval_regtype_tbl_size = 1<<16,
    rz3_tdata_regval_regid_tbl_size = 1<<16,
    rz3_tdata_itlb_size = 512,
    rz3_tdata_dtlb_size = 2048,
    rz3_tdata_rfs_pc_pred_table_size = 1<<16
  };


  rz3_percpu_data(int32_t arg_cpuid);
  ~rz3_percpu_data();
  void clear();


}; // rz3_percpu_data


#endif // _rz3_section_h_
