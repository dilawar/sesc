#the following line has to be before the include
SIGNATURE=libapp
include	../Makefile.defs

##############################################################################
.PHONY: all clean

TESTS=thr_tst1 thr_tst2 t1 t2 t3 t4 t5 ts_tst1 ts_tst2

ifdef TLS
TESTS +=
endif

ifdef TASKSCALAR
TESTS += ts_tst1 ts_tst2
endif

ifndef IACOMA1
tests	: 	 $(TESTS)
endif

all	:	$(OBJ)/libapp.a

tls_test:	$(OBJ)/tls_test.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

tls_loop_test:	$(OBJ)/tls_loop_test.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

tls_sync_test:	$(OBJ)/tls_sync_test.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

ts_tst1:	$(OBJ)/ts_tst1.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

ts_tst2:	$(OBJ)/ts_tst2.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

thr_tst2:	$(OBJ)/thr_tst2.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

thr_tst1:	$(OBJ)/thr_tst1.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

t1	:	$(OBJ)/t1.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

t2	:	$(OBJ)/t2.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

t3	:	$(OBJ)/t3.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

t4	:	$(OBJ)/t4.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

t5	:	$(OBJ)/t5.o $(OBJ)/libapp.a
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

event_tst1:	$(OBJ)/event_tst1.o $(OBJ)/libapp.a 
		$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

##############################################################################
#                Objects
##############################################################################
OBJS	:= sesc_thread.o sesc_events.o sesc_locks.o sesc_fortran.o sesc_chud.o

ifdef TLS
OBJS   += sesc_tls.o
endif

ifdef TASKSCALAR
# It provides minor compatibility with VERSIONMEM interface
OBJS   += sesc_ts.o
endif

##############################################################################
#                             Change Rules                                   # 
##############################################################################

# eclean is called by clean
eclean:
	-@rm -f $(DOTDEPEND)
	-@rm -f $(OBJ)/libapp.a tm
	-@rm -f $(TESTS) $(patsubst %,$(OBJ)/%.o,$(TESTS))

size:
	-@wc *.[chp]

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
