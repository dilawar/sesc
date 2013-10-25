#the following line has to be before the include
SIGNATURE=libll
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default: all

all: $(OBJ)/ta.o $(OBJ)/main.o $(OBJ)/libll.a $(OBJ)/sta.o	

##############################################################################
#                Objects
##############################################################################
OBJS	:=Instruction.o MIPSInstruction.o PPCInstruction.o GFlow.o \
	ExecutionFlow.o Events.o ThreadContext.o HeapManager.o TraceReader.o \
	TraceFlow.o TT6Reader.o QemuSescReader.o  

ifdef QEMU_DRIVEN
OBJS += QEMUFlow.o QemuSparcInstruction.o SPARCInstruction.o
endif

ifdef SESC_RSTTRACE
OBJS += RSTIntruction.o RSTReader.o RSTFlow.o SPARCInstruction.o
endif

##############################################################################


##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libll.a
	-@rm -f $(OBJ)/main.o

size:
	-@wc *.cpp *.h *.hpp

_MAKEFILE_COMMON_SUBSHELL=1

include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
# DO NOT DELETE
