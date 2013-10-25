#the following line has to be before the include
SIGNATURE=libemul
include	../Makefile.defs
##############################################################################
.PHONY: all

all	: $(OBJ)/libemul.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= AddressSpace.o EmulInit.o ElfObject.o \
           InstDesc.o LinuxSys.o SignalHandling.o FileSys.o \
           X86InstDesc.o 
##############################################################################
#                             Change Rules                                   # 
##############################################################################
.PHONY: eclean
eclean:
	-@rm -f $(OBJ)/libemul.a 2>/dev/null

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
