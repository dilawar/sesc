#the following line has to be before the include
SIGNATURE=libnet
include ../Makefile.defs

##############################################################################
.PHONY: all clean tests

TESTS	=test1 test2 tstProt random rpTest1 netBench

all	: $(OBJ)/libnet.a 

##############################################################################
#                            Test Examples                                   # 
##############################################################################
tests	: $(TESTS)

netBench: $(OBJ)/netBench.o $(SESCENV) $(NETWORK) $(SESCENV2)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

rpTest1	: $(OBJ)/rpTest1.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

test1	: $(OBJ)/test1.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

test2	: $(OBJ)/test2.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

tstProt	: $(OBJ)/tstProt.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

scenario1: $(OBJ)/scenario1.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

random: $(OBJ)/random.o $(SESCENV) $(SESCENV2) $(NETWORK)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -lm

##############################################################################
#                Objects
##############################################################################
OBJS	:=InterConn.o Message.o Router.o ProtocolBase.o PMessage.o RoutingTable.o RoutingPolicy.o NetAddrMap.o
##############################################################################
#                             Change Rules                                   # 
##############################################################################
eclean:
	-@rm -f $(OBJ)/libnet.a
	-@rm -f *.log
	-@rm -f $(TESTS) $(patsubst %,$(OBJ)/%.o,$(TESTS))
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
