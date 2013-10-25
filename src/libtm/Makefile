#the following line has to be before the include
SIGNATURE=libtm
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/TMmain.o $(OBJ)/libtm.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= TMCache.o TMSystemBus.o TMMemorySystem.o 
OBJS    += MESIProtocol.o TMProtocol.o TMMemRequest.o 

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libtm.a
	-@rm -f $(OBJ)/TMmain.o

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
