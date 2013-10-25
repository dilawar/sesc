#the following line has to be before the include
SIGNATURE=libsuperlu
include	../Makefile.defs
##############################################################################
.PHONY: all clean tests

all	: $(OBJ)/libsuperlu.a

##############################################################################
#                Objects
##############################################################################
# Single
#OBJS:= (SLUSRC) $(ALLAUX) $(LAAUX) $(SLASRC) $(SCLAUX)
# double
OBJS:= dgssv.o dgssvx.o dsp_blas2.o dsp_blas3.o dgscon.o dlangs.o dgsequ.o dlaqgs.o dpivotgrowth.o dgsrfs.o dgstrf.o dgstrs.o dcopy_to_ucol.o dsnode_dfs.o dsnode_bmod.o dpanel_dfs.o dpanel_bmod.o dreadhb.o dcolumn_dfs.o dcolumn_bmod.o dpivotL.o dpruneL.o dmemory.o dutil.o dmyblas2.o superlu_timer.o su_util.o memory.o get_perm_c.o mmd.o sp_coletree.o sp_preorder.o sp_ienv.o relax_snode.o heap_relax_snode.o colamd.o lsame.o xerbla.o dlacon.o dlamch.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
.PHONY: all clean

eclean:
	-@rm -f $(OBJ)/libsuperlu.a

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


