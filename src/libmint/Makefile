#the following line has to be before the include
SIGNATURE=libmint
include	../Makefile.defs
##############################################################################
.PHONY: all

all	: $(OBJ)/libmint.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= mint_init.o opcodes.o dump.o exec.o subst.o error_ops.o symtab.o \
	ops.o coproc.o utils.o elf.o mendian.o non_mips.o icode.o
ifdef ADDRESS_SPACES
OBJS    += AddressSpace.o
endif
##############################################################################
#                             Change Rules                                   # 
##############################################################################
.PHONY: eclean
eclean:
	-@rm -f $(OBJ)/libmint.a 2>/dev/null

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
