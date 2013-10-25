#the following line has to be before the include
SIGNATURE=libmem
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/mtst1.o $(OBJ)/libmem.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= MemorySystem.o Cache.o Bank.o MemCtrl.o Bus.o MemoryOS.o  \
           TLB.o StridePrefetcher.o UglyMemRequest.o AddressPrefetcher.o \
	   PriorityBus.o

ifdef TS_CAVA
OBJS	+= MValuePredictor.o
endif
##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libmem.a
	-@rm -f $(OBJ)/mtst1.o

size:
	-@wc *.cpp *.h

_MAKEFILE_COMMON_SUBSHELL=1

include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
# DO NOT DELETE
