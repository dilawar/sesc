# makefile for sparse supernodal LU, implemented in ANSI C, targeted for 
# shared memory machines.
#
include ../make.inc

#######################################################################
#  This makefile creates the example programs for the linear equation
#  routines in SuperLU.
#
#  The command
#       make
#  without any arguments creates all the example programs.
#  The executable files are called
#       pdlinsol	
#	pdlinsolx
#	pdrepeat
#	pdspmd
#
#  To remove the object files after the executable files have been
#  created, enter
#       make clean
#######################################################################

HEADER = ../SRC

all: pdlinsol pdlinsolx pdrepeat pdspmd

DLINOBJS 	= pdlinsol.o
DLINXOBJS 	= pdlinsolx.o
DREPOBJS	= pdrepeat.o
DSPMDOBJS	= pdspmd.o 

pdlinsol: $(DLINOBJS) ../$(SUPERLULIB)
	$(LOADER) $(LOADOPTS) $(DLINOBJS) \
	$(BLASLIB) $(MATHLIB) $(MPLIB) -o $@ $(VERBOSE)

pdlinsolx: $(DLINXOBJS) ../$(SUPERLULIB)
	$(LOADER) $(LOADOPTS) $(DLINXOBJS) \
	$(BLASLIB) $(MATHLIB) $(MPLIB) -o $@ $(VERBOSE)

pdrepeat: $(DREPOBJS) ../$(SUPERLULIB)
	$(LOADER) $(LOADOPTS) $(DREPOBJS) \
	$(BLASLIB) $(MATHLIB) $(MPLIB) -o $@ $(VERBOSE)

pdspmd: $(DSPMDOBJS) ../$(SUPERLULIB)
	$(LOADER) $(LOADOPTS) $(DSPMDOBJS) \
	$(BLASLIB) $(MATHLIB) $(MPLIB) -o $@ $(VERBOSE)

F77EXM	= f77_main.o hbcode1.o c_bridge_pdgssv.o

f77exm: $(F77EXM) ../$(SUPERLULIB)
	$(FORTRAN) $(LOADOPTS) $(F77EXM) \
	../$(SUPERLULIB) $(BLASLIB) $(MATHLIB) $(MPLIB) -o $@

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) -I$(HEADER) -c $< $(VERBOSE)

.f.o:
	$(FORTRAN) $(FFLAGS) -c $< $(VERBOSE)

clean:	
	rm -f *.o core pdlinsol pdlinsolx pdrepeat pdspmd f77exm

