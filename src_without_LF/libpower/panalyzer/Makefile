#the following line has to be before the include
SIGNATURE=libpanalyzer
include	../../Makefile.defs
##############################################################################
.PHONY: all clean

all	: $(DOTDEPEND) $(OBJS) $(OBJ)/libpanalyzer.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= technology.o circuit.o logic.o support.o microstrip.o clock.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libpanalyzer.a
	-@rm -f $(patsubst %.o,$(OBJ)/%.o,$(OBJS))

_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND) 
endif
endif

# DO NOT DELETE
