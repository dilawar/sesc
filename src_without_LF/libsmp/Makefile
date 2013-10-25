#the following line has to be before the include
SIGNATURE=libsmp
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/smp.o $(OBJ)/libsmp.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= SMPCache.o SMPSystemBus.o SMemorySystem.o 
OBJS    += MESIProtocol.o SMPProtocol.o SMPMemRequest.o 

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libsmp.a
	-@rm -f $(OBJ)/smp.o

size:
	-@wc *.cpp *.h

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
