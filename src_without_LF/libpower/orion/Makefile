#the following line has to be before the include
SIGNATURE=liborion
include	../../Makefile.defs
##############################################################################
.PHONY: all clean

all	: $(DOTDEPEND) $(OBJ)/liborion.a

##############################################################################
#               Definitions
##############################################################################

# IBM SA-27 technology (InfiniBand)
#DEFS   += -DPOWER_TEST -DPARM_TECH_POINT=18 -DVdd=1.8 -DPARM_Freq=2.5e8
# IBM SA-27E technology (RAW)
#DEFS   += -DPOWER_TEST -DPARM_TECH_POINT=11 -DVdd=1.8 -DPARM_Freq=3e8
# IBM Cu-11 technology
#DEFS   += -DPOWER_TEST -DPARM_TECH_POINT=11 -DVdd=1.2 -DPARM_Freq=2.5e8

# for general testing
#DEFS   += -DPOWER_TEST -DPARM_TECH_POINT=7 -DVdd=1.1 -DPARM_Freq=5e9


##############################################################################
#                Objects
##############################################################################
ARRAY_MODEL  = SIM_power_array_l.c SIM_power_array_m.c SIM_power_cam.c
ROUTER_MODEL = SIM_power_router.c
ALU_MODEL    = SIM_power_ALU.c
OTHER_MODEL  = SIM_power_misc.c SIM_power_permu.c
MISC_SRC     = SIM_power_util.c SIM_power_time.c
INTERFACE_SRC = orion_setup.cpp
SRCS         = $(ARRAY_MODEL) $(ROUTER_MODEL) $(ALU_MODEL) $(OTHER_MODEL) \
               $(MISC_SRC) 
OBJS	:= $(SRCS:.c=.o) SIM_router_power.o orion_setup.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/liborion.a
	-@rm -f $(patsubst %.o,$(OBJ)/%.o,$(OBJS))

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif

