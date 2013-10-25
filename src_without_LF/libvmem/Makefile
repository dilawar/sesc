#the following line has to be before the include
SIGNATURE=libvmem
include ../Makefile.defs

##############################################################################
.PHONY: all clean 

VMEMLIB	:=$(OBJ)/libvmem.a

default : all

all	: $(OBJ)/tls.o $(VMEMLIB) 

power.conf: sesc.conf shared.conf ../libpower/wattchify ../libpower/cactify
	../libpower/wattchify sesc.conf tmp.conf
	../libpower/cactify tmp.conf power.conf


##############################################################################
#                Objects
##############################################################################
OBJS	 = LVIDTable.o LMVCache.o FMVCache.o GMVCache.o MVCacheState.o 
OBJS	+= VCache.o VCR.o VBus.o VTaskHandler.o 
OBJS	+= VMemObj.o VMemorySystem.o VMemReq.o VMemState.o 
OBJS    += DirState.o Directory.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
eclean:
	-@rm -f $(VMEMLIB) $(OBJ)/tls.o
	-@rm -f *.log
	-@rm -f net

size:
	wc *.cpp *.h

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
