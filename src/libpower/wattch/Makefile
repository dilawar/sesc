#the following line has to be before the include
SIGNATURE=libwattch
include	../../Makefile.defs
##############################################################################
.PHONY: all clean

all	: $(DOTDEPEND) $(OBJS) $(OBJ)/libwattch.a
##############################################################################
#                Objects
##############################################################################
OBJS = wattch_power.o wattch_setup.o wattch_time.o wattch_io.o wattch_cache.o wattch_misc.o
##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libwattch.a 
	-@rm -f $(patsubst %.o,$(OBJ)/%.o,$(OBJS))

_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif

# DO NOT DELETE
