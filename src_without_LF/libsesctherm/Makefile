#the following line has to be before the include
SIGNATURE=libsesctherm
include	../Makefile.defs
##############################################################################
.PHONY: all clean tests

all	: $(OBJ)/libsesctherm.a

##############################################################################
#                Objects
##############################################################################
OBJS:=sesctherm.o sesctherm3Dbase.o sesctherm3Dchip.o sesctherm3Dconfig.o sesctherm3Ddatalibrary.o

OBJS+=sesctherm3Dlayerinfo.o sesctherm3Dmaterial.o sesctherm3Dmodel.o sesctherm3Ducool.o sesctherm3Dutil.o

OBJS+=sesctherm3Dgraphics.o sesctherm3Dregression.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
.PHONY: all clean

eclean:
	-@rm -f $(OBJ)/libsesctherm.a

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


