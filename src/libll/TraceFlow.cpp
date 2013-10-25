/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
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

#include "TraceFlow.h"
#include "OSSim.h"
#include "SescConf.h"
#include "TT6Reader.h"
#include "QemuSescReader.h"
#ifdef SESC_SIMICS
#include "SimicsReader.h"
#endif

char *TraceFlow::traceFile = 0;
TraceReader *TraceFlow::trace = 0;
TraceFlow::TraceFlow(int32_t cId, int32_t i, GMemorySystem *gms) 
  : GFlow(i, cId, gms)
{
#if (defined(TASKSCALAR) || defined(SESC_MISPATH))
  MSG("TraceFlow::TASKSCALAR or SESC_MISPATH not supported yet");
  exit(-5);
#endif
  bool createReader = (!trace);
  // all traceflow instances share the same reader obj
  
  const char *traceMode = SescConf->getCharPtr("","traceMode");
  
  // FIXME: do not use traceMode. Try to discover with file name
  if(strcasecmp(traceMode, "ppctt6") == 0) {
    if(createReader) {
      trace = new TT6Reader();
    }
    mode = PPCTT6Trace;
  }else if (strcmp (traceMode,"qemusparc")==0) {
    if (createReader) {
      trace = new QemuSescReader(); 
    }
    mode = QemuSpTrace;
  } else if(strcmp(traceMode, "simics") == 0) {

#ifdef SESC_SIMICS
    if(createReader)
      trace = new SimicsReader();
#else
    MSG("Simics mode not fully supported yet. Sorry.");
    exit(0);
#endif

    mode = SimicsTrace;
  } else {
      I(0);
  }

  if(createReader) {
    I(traceFile);
    MSG("TraceFlow::TraceFlow() traceFile=%s", traceFile);
    trace->openTrace(traceFile);
  }
  
  nextPC     = 0;
  hasTrace   = true;

  delayDInst        = 0;
  swappingDelaySlot = false;
}

DInst *TraceFlow::executePC() 
{ 
  I(hasTrace);
  static TraceEntry te; // static for speed, otherwise constructor is called every time

  trace->fillTraceEntry(&te, fid);
        
  if(te.eot) { // end of trace
    hasTrace = false; // FIXME: remove 
    return 0;
  }

  const Instruction *inst = Instruction::getSharedInstByPC(te.iAddr);

  if (inst ==0) {
    // Instruction is still not predecoded
    switch (mode) {
    case QemuSpTrace:
      inst = Instruction::getQemuInstByPC(te.iAddr, static_cast<QemuSescReader *>(trace)->currentInst());
      break;

    case PPCTT6Trace:
      inst = Instruction::getPPCInstByPC(te.iAddr, te.rawInst);    
      break;
      
    case SimicsTrace:
#ifdef SESC_SIMICS
      inst = Instruction::getSimicsInst((TraceSimicsOpc_t) te.rawInst);
#else
      inst = 0; //avoiding warning
#endif
      break;

    default:
      I(0);
    }
  }

  nextPC = te.nextIAddr;

  DInst *dinst=DInst::createDInst(inst, te.dAddr ,cpuId
#if (defined MIPS_EMUL)
				  ,0 // This will break things (context can't be 0)
#endif
#if (defined TLS)
                            ,0 // This will break things (epoch can't be 0)
#endif
                            );

  return dinst;
}

void TraceFlow::dump(const char *str) const
{
  MSG("TraceFlow::dump() not implemented. stop being lazy and do it!");
}

