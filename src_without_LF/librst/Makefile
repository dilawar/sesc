#the following line has to be before the include
SIGNATURE=librst
include	../Makefile.defs
##############################################################################
.PHONY: all clean tests

all	: $(OBJ)/librst.a

##############################################################################
#                Objects
##############################################################################
OBJS := Rstzip.o cpuid.o rstzip2if.o rz3_section.o compress_engine.o 
OBJS += decompress_engine.o rstzip3.o rz3iu.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
.PHONY: all clean

eclean:
	-@rm -f $(OBJ)/librst.a

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


