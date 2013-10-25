#the following line has to be before the include
SIGNATURE=libsuc
include	../Makefile.defs
##############################################################################
.PHONY: all clean tests

all	: $(OBJ)/libsuc.a

##############################################################################
#                            Test Examples                                   # 
##############################################################################
TESTS	=poolBench Config_tst callback_tst CacheSample TQtest1

poolBench: $(OBJ)/poolBench.o $(OBJ)/libsuc.a
	$(CXX) -o $(@F) $< $(OBJ)/libsuc.a

TQtest1: $(OBJ)/TQtest1.o $(OBJ)/libsuc.a
	$(CXX) -o $(@F) $< -L$(OBJ) -lsuc

Config_tst:	$(OBJ)/Config_tst.o $(OBJ)/libsuc.a
	$(CXX) -o $(@F) $< -L$(OBJ) -lsuc

callback_tst:	$(OBJ)/callback_tst.o $(OBJ)/libsuc.a
	$(CXX) -o $(@F) $< -L$(OBJ) -lsuc

CacheSample:	$(OBJ)/CacheSample.o $(OBJ)/libsuc.a
	$(CXX) -o $(@F) $< -L$(OBJ) -lsuc

##############################################################################
#                Objects
##############################################################################
SOBJS	:= TQueue.o Config.o nanassert.o GStats.o callback.o \
	Snippets.o Port.o ReportGen.o CacheCore.o SescConf.o \
	TraceGen.o SCTable.o BloomFilter.o 


ifdef SESC_ENERGY
SOBJS	+= GEnergy.o
endif

ifdef SESC_THERM
SOBJS	+= ReportTherm.o ThermTrace.o
endif

ifdef SESC_CRIT
SOBJS   += CriticalityGraph.o CriticalityPred.o
endif

# Objects from generated C/C++ files 
GOBJS	:=conflex.tab.o conflexlex.o

OBJS	:= $(GOBJS) $(SOBJS)
##############################################################################
#                             Change Rules                                   # 
##############################################################################
eclean:
	-@rm -f Config_tst.log
	-@rm -f $(OBJ)/libsuc.a
	-@rm -f $(OBJ)/conflex.tab.cpp.h $(OBJ)/conflex.tab.hpp $(OBJ)/conflex.tab.cpp $(OBJ)/conflexlex.cpp 
	-@rm -f $(OBJ)/conflex.tab.o $(OBJ)/conflexlex.o
	-@rm -f $(TESTS) $(patsubst %,$(OBJ)/%.o,$(TESTS))

size:
	wc *.cpp *.h *.l *.y

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
