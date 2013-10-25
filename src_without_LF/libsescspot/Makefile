#the following line has to be before the include
SIGNATURE=libsescspot
include	../Makefile.defs
##############################################################################
.PHONY: all clean

all	: $(OBJ)/libsuc.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= FBlocks.o Grids.o RCutil.o flp.o flp_desc.o
OBJS	+= npe.o sescspot.o shape.o temperature.o temperature_block.o
OBJS	+= temperature_grid.o util.o wire.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
eclean:
	-@rm -f $(OBJ)/libsescspot.a
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
