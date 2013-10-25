/*
 * Copyright 1998-2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SUN MICROSYSTEMS, INC.
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "InstType.h"
#include "SPARCInstruction.h"

#include "rstf.h"
#include "Rstzip.h"

const char usage[] = "rstexample <input-trace-file>";

int32_t main(int32_t argc, char **argv)
{
  // argv[1] must be input file
  const char * ifname = NULL;

  int32_t i=1;
  while(i<argc) {
    const char * arg = argv[i++];
    if (strcmp(arg, "-h") == 0) {
      printf("Usage: %s\n", usage);
      exit(0);
    } else if (ifname != NULL) {
      fprintf(stderr, "ERROR: rstexample: input file %s already specified\nUsage: %s\n", ifname, usage);
      exit(1);
    } else {
      ifname = arg;
    }
  }

  if (ifname == NULL) {
    printf("Usage: %s\n", usage);
    exit(0);
  }

  // create an rstzip instance
  Rstzip * rz = new Rstzip;
  int32_t rv=rz->open(ifname, "r", "verbose=0");
  if (rv != RSTZIP_OK) {
    fprintf(stderr, "ERROR: rstexample: Rstzip error opening input file %s\n", ifname);
    exit(1);
  }

  const int32_t max_ncpu=1<<10; // RST supports 10-bit cpuids
  int64_t icounts[max_ncpu]; 
  memset(icounts, 0, max_ncpu*sizeof(int64_t));
  int64_t total_icount = 0;

  int32_t nrecs;
  rstf_unionT buf[rstzip_opt_buffersize];
  while((nrecs = rz->decompress(buf, rstzip_opt_buffersize)) != 0) {
    int32_t i;
    for (i=0; i<nrecs; i++) {
      rstf_unionT * rp = buf+i;
      if (rp->proto.rtype == INSTR_T) {
	total_icount++;
	int32_t cpuid = rstf_instrT_get_cpuid(&(rp->instr));
	icounts[cpuid]++;
	
	InstType     type;
	InstSubType  subType;
	uint32_t rd;
	uint32_t rs1;
	uint32_t rs2;
	disas_sparc_insn(rp->instr.instr,
			 type, 
			 subType,
			 rd,
			 rs1,
			 rs2);
	printf("cpu=%d PC=0x%8llx LD=0x%8llx opcode=0x%x type=%d subType=%d %d = %d ? %d\n"
	       ,cpuid, rp->instr.pc_va, rp->instr.ea_va, rp->instr.instr
	       ,type, subType, rd, rs1, rs2
	       );
#if 0
      }else if(rp->proto.rtype == REGVAL_T) {
	int32_t cpuid = rstf_regvalT_get_cpuid(&(rp->regval));
	printf("cpu=%d src1(%d,%d) src2(%d,%d)\n", cpuid
	       ,rp->regval.regtype[0], rp->regval.regid[0]
	       ,rp->regval.regtype[1], rp->regval.regid[1]
	       );
#endif
      }
      
    }
  }
  rz->close();
  delete rz;
  rz=NULL;

  printf("Total icount=%lld\n", total_icount);
  for (i=0; i<max_ncpu; i++) {
    if (icounts[i] > 0) {
      printf("cpu%d: icount=%lld\n", i, icounts[i]);
    }
  }
} // main()
