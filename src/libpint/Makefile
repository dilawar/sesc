#the following line has to be before the include
SIGNATURE=libpint
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default: all

all: 

##############################################################################
#                Objects
##############################################################################
OBJS	:= PPCDecoder.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libpint.a
size:
	-@wc *.cpp *.h *.hpp

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
