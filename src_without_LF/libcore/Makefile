#the following line has to be before the include
SIGNATURE=libcore
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default: all

all: $(OBJ)/main.o $(OBJ)/libcore.a 

##############################################################################
#                Objects
##############################################################################
OBJS	:=Processor.o GProcessor.o LDSTQ.o SMTProcessor.o DInst.o Pipeline.o \
	FetchEngine.o Resource.o Cluster.o DepWindow.o BPred.o \
	MemRequest.o MemObj.o  OSSim.o LDSTBuffer.o \
	ProcessId.o RunningProcs.o GMemorySystem.o ValueTable.o \
	GMemoryOS.o VPred.o 


ifdef SESC_INORDER
OBJS += Signature.o
endif


ifdef TLS
OBJS	+=Epoch.o Checkpoint.o AdvancedStats.o SysCall.o
endif

ifdef TASKSCALAR
ifdef TLS
$(error You can not compile TaskScalar simultaneously with TLS)
endif
OBJS	+=TaskContext.o TaskHandler.o MemBuffer.o GLVID.o

ifdef ATOMIC
OBJS    +=ASVersion.o
else
OBJS    +=HVersion.o
endif

ifdef TS_PROFILING
OBJS	+=Profile.o
endif

ifdef TS_RISKLOADPROF
OBJS	+=RiskLoadProf.o
endif

endif

ifdef SESC_ENERGY
OBJS   += EnergyMgr.o
endif

ifdef SESC_SLICE
OBJS  += Slicer.o
endif

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libcore.a
	-@rm -f $(OBJ)/main.o

size:
	-@wc *.cpp *.h *.hpp

_MAKEFILE_COMMON_SUBSHELL=1

include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
# DO NOT DELETE
