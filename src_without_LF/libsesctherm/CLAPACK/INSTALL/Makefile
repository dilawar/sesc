include ../make.inc
F2CLIB = ../F2CLIBS/libF77.a ../F2CLIBS/libI77.a

all:  testlsame testslamch testdlamch testsecond testdsecnd testieee

testlsame:  lsame.o lsametst.o
	$(LOADER) $(LOADOPTS) -o testlsame lsame.o lsametst.o \
	$(F2CLIB) -lm -lc

testslamch: slamch.o lsame.o slamchtst.o
	$(LOADER) $(LOADOPTS) -o testslamch slamch.o lsame.o slamchtst.o \
	$(F2CLIB) -lm -lc

testdlamch: dlamch.o lsame.o dlamchtst.o
	$(LOADER) $(LOADOPTS) -o testdlamch dlamch.o lsame.o dlamchtst.o \
	$(F2CLIB) -lm -lc

testsecond: second.o secondtst.o
	$(LOADER) $(LOADOPTS) -o testsecond second.o secondtst.o \
	$(F2CLIB) -lm -lc

testdsecnd: dsecnd.o dsecndtst.o
	$(LOADER) $(LOADOPTS) -o testdsecnd dsecnd.o dsecndtst.o \
	$(F2CLIB) -lm -lc

testieee: tstiee.o
	$(LOADER) $(LOADOPTS) -o testieee tstiee.o \
	$(F2CLIB) -lm -lc

clean:
	rm -f *.o

slamch.o: slamch.c ; $(CC) $(NOOPT) -c $<
dlamch.o: dlamch.c ; $(CC) $(NOOPT) -c $<

.c.o: ; $(CC) $(CFLAGS) -c $<

