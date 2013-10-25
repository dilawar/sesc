#the following line has to be before the include
SIGNATURE=libcacti
include	../../Makefile.defs
##############################################################################
.PHONY: all clean

all	: $(DOTDEPEND) $(OBJ)/libcacti.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= cacti42_area.o cacti42_basic_circuit.o cacti42_io.o cacti42_leakage.o cacti42_time.o
OBJS	+= cacti_setup.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libcacti.a
	-@rm -f $(patsubst %.o,$(OBJ)/%.o,$(OBJS))

_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND) 
endif
endif

# DO NOT DELETE
