# Makefile for GFAST/src/activeMQ
#
EEWDIR = ../../..
GFAST_DIR = ../..
# Make.include.gfast will include $(EEWDIR)/Make.include.$(shell uname)
include $(GFAST_DIR)/Make.include.gfast

OBJS	= activeMQ.c.o activeMQ.cpp.o readIni.c.o
LIB	= gfast_amq.a
DEBUG	= -g
#CCFLAGS	+= -std=c++0x -O0 -D_REENTRANT -Dstatic_config

INCL= -I ../../include $(APR_INCL) $(ACTIVEMQ_INCL) $(INIPARSER_INCL) $(ISCL_INCL)

all: $(LIB)
lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(LIB) $(OBJS)

%.c.o:    %.c
	$(cc) -c $< -o $@ $(CFLAGS) $(DEBUG) $(INCL)
%.cpp.o:    %.cpp
	$(CXX) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)

ids:    *.c *cpp *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.c *cpp *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)

clean:
	-rm -f *.o $(LIB)

veryclean: clean rm-ids cleandepend

install: #no-op

# coverage
cleancoverage: clean
	-rm -rf *.gcno *.gcda *.gcov *.info

buildcoverage: prepcoverage all

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval CFLAGS=${CFLAGS} -fprofile-arcs -ftest-coverage)
