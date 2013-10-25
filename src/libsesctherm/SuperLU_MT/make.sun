############################################################################
#
#  Program:         SuperLU_MT
#
#  Module:          make.inc
#
#  Purpose:         Top-level Definitions
#
#  Creation date:   August 15, 1997
#
#  Modified:	    September 1, 1999 version 1.0
#
############################################################################
#
#  The machine (platform) identifier to append to the library names
#
PLAT = _SOLARIS

# Common Libraries
TMGLIB       	= tmglib$(PLAT).a
SUPERLULIB   	= superlu_mt$(PLAT).a
BLASLIB     	= ../blas$(PLAT).a
MATHLIB	     	= -lm 
MPLIB		= -lthread

#
#  The archiver and the flag(s) to use when building archive (library)
#  If you system has no ranlib, set RANLIB = echo.
#
ARCH         	= ar
ARCHFLAGS    	= cr
RANLIB       	= ranlib

# for Solaris 2.x
CC	     	= cc
#
# Definitions used by CPP:
PREDEFS      	= -D$(PLAT)
#
CFLAGS       	= $(PREDEFS) -xO3 -xcg92 $(DBG)
NOOPTS	     	=
BLASOPTS	=
LOADER       	= cc
LOADOPTS     	= -O
FORTRAN		= f77
FFLAGS		= -O

#
#  C preprocessor defs for compilation (-DNoChange, -DAdd_, or -DUpCase)
#
CDEFS        	= -DAdd_
#
# The following check only works with gcc
# VERBOSE      	= -Wunused -Wuninitialized
