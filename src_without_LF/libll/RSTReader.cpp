/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2005 University California, Santa Cruz.

   Contributed by Saangetha
                  Keertika
                  Jose Renau

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <sys/types.h>
#include <dirent.h>

#include "SescConf.h"
#include "OSSim.h"
#include "nanassert.h"
#include "RSTReader.h"
#include "rstf.h"
#include "Rstzip.h"

void RSTReader::addInstruction(const rstf_unionT *rp) {

  int32_t fid       = rstf_instrT_get_cpuid(&(rp->instr));
 
  VAddr PC      = rp->instr.pc_va;
  uint32_t  rawInst = rp->instr.instr;
  VAddr address = rp->instr.ea_va;

  const Instruction *inst = Instruction::getSharedInstByPC(PC);
  if (inst == 0) {
    inst = Instruction::getRSTInstByPC(PC, rawInst);
    GI(inst->isMemory(), address>1024 || address==0);
  }
  I(inst);

  DInst *dinst=DInst::createDInst(inst, address , fid
#ifdef TLS
                                  ,0 // This will break things (epoch can't be 0)
#endif
                                  );

  I(fid<nFlows);

  head[fid].addSrc1(dinst);

  head_size[fid]++;
}

void RSTReader::advancePC(int32_t fid) { 

  while(!end_of_trace) {
    while (buf_pos < buf_end) {
      rstf_unionT *rp = &buf[buf_pos++];

      // skip markers, only INSTR_T
      if (rp->proto.rtype != INSTR_T)
        continue;

      int32_t inst_fid       = rstf_instrT_get_cpuid(&(rp->instr));
      if (inst_fid >= nFlows) {
        static int32_t max_flow_found = nFlows;
        if(max_flow_found < inst_fid) {
          MSG("More Flows (%d) than thread contexts (%d)",inst_fid, nFlows);
          max_flow_found = inst_fid;
        }
        continue;
      }
      if( head_size[inst_fid] > Max_Head_Size) {
        buf_pos--;
        // Too many instruction on the other context

        if(head_size[fid]==0) {
          // stop current fid (stopcpu)
          osSim->stopProcessor(fid);
        }
        return;
      }

      addInstruction(rp);

      // if instruction from another thread add it to the list
      if(rstf_instrT_get_cpuid(&(rp->instr))!=fid && head_size[inst_fid]==1) {
        osSim->restartProcessor(inst_fid);
        continue;
      }
      
      return;
    }

    // Fill Buffer
    I(buf_pos == buf_end);

    buf_end = rz->decompress(buf, Max_Num_Recs);
    buf_pos = 0;
    if (buf_end == 0) {
      end_of_trace = true;
    }
  }
}

RSTReader::RSTReader()
  : Max_Num_Recs(128)
  , Max_Head_Size(32) {

  buf = (rstf_unionT *)malloc(sizeof(rstf_unionT)*Max_Num_Recs);
  buf_pos = 0;
  buf_end = 0;

  int32_t nProcs = SescConf->getRecordSize("","cpucore");
  nFlows = 0;
  for(Pid_t i = 0; i < nProcs; i ++) {
    if(SescConf->checkInt("cpucore","smtContexts",i)) {
      nFlows += SescConf->getInt("cpucore","smtContexts",i);
    }else{
      nFlows++;
    }
  }

  head = new DInst [nFlows];
  head_size = (char *)malloc(sizeof(char)*nFlows);
  bzero(head_size, sizeof(char)*nFlows);
}

void RSTReader::openTrace(const char *filename) {

  rz = new Rstzip;
  int32_t rv=rz->open(filename, "r", "verbose=0");
  if (rv != RSTZIP_OK) {
    MSG("ERROR: RSTReader::openTrace(%s) error opening input trace", filename);
    exit(1);
  }

  end_of_trace = false;

  while(!end_of_trace) {
    while (buf_pos < buf_end) {
      rstf_unionT *rp = &buf[buf_pos++];

      // skip markers, only INSTR_T
      if (rp->proto.rtype != INSTR_T)
        continue;

      int32_t fid       = rstf_instrT_get_cpuid(&(rp->instr));

      VAddr PC      = rp->instr.pc_va;
      uint32_t  rawInst = rp->instr.instr;
      VAddr address = rp->instr.ea_va;

      const Instruction *inst = Instruction::getSharedInstByPC(PC);
      if (inst == 0) {
        inst = Instruction::getRSTInstByPC(PC, rawInst);
        GI(inst->isMemory(), address>1024 || address==0);
      }
      I(inst);

      DInst *dinst=DInst::createDInst(inst, address , fid
#ifdef TLS
                                  ,0 // This will break things (epoch can't be 0)
#endif
                                  );

      fid = fid % nFlows;
      head[fid].addSrc1(dinst);

      head_size[fid]++;

      return; // one instruction added :)
    }

    // Fill Buffer
    I(buf_pos == buf_end);

    buf_end = rz->decompress(buf, Max_Num_Recs);
    buf_pos = 0;
    if (buf_end == 0)
      end_of_trace = true;
  }
}

void RSTReader::closeTrace() {
  rz->close();
  delete rz;
}

DInst *RSTReader::executePC(int32_t fid) {

  I(fid<nFlows);

  GI( head_size[fid],  head[fid].hasPending());
  GI(!head_size[fid], !head[fid].hasPending());

  I((Max_Head_Size/4)>4);
  if (head_size[fid] < (Max_Head_Size/4))
    advancePC(fid);

  if (head_size[fid]==0)
    return 0;

  head_size[fid]--;
  DInst *dinst = head[fid].getNextPending();

  if (head_size[fid]==0)
    advancePC(fid);

  return dinst;
}
