include ../make.inc

#######################################################################
#  This makefile creates the test programs for the linear equation
#  routines in SuperLU.  The test files are grouped as follows:
#
#       ALINTST -- Auxiliary test routines
#       SLINTST -- Single precision real test routines
#       DLINTST -- Double precision real test routines
#       CLINTST -- Double precision complex test routines
#       ZLINTST -- Double precision complex test routines
#
#  Test programs can be generated for all or some of the four different
#  precisions.  Enter make followed by one or more of the data types
#  desired.  Some examples:
#       make single
#       make single double
#  Alternatively, the command
#       make
#  without any arguments creates all four test programs.
#  The executable files are called
#       plintsts
#       plintstd
#       plintstc
#       plintstz
#
#  To remove the object files after the executable files have been
#  created, enter
#       make clean
#  On some systems, you can force the source files to be recompiled by
#  entering (for example)
#       make single FRC=FRC
#
#######################################################################

HEADER  = ../SRC

ALINTST = sp_ienv.o

DLINTST = pddrive.o sp_dconvert.o pdgst01.o pdgst02.o pdgst04.o pdgst07.o \
	  pdgssv.o pdgssvx.o

double: ./plintstd pdtest.out

./plintstd: $(DLINTST) $(ALINTST) ../$(SUPERLULIB) $(TMGLIB)
	$(LOADER) $(LOADOPTS) $(DLINTST) $(ALINTST) \
        $(TMGLIB) ../$(SUPERLULIB) $(BLASLIB) -lm $(MPLIB) -o $@

pdtest.out: plintstd pdtest.csh
	@echo Testing DOUBLE PRECISION linear equation routines 
	csh pdtest.csh

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) -I$(HEADER) -c $< $(VERBOSE)

clean:	
	rm -f *.o plintst* *.out

