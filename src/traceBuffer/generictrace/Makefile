# Makefile for GFAST/src/traceBuffer/generictrace
#
GFASTROOT=../..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= freeGenerictrace.o freeGenerictraceData.o setGenerictraceDataFromGFAST.o \
generictrace_hash.o printGenerictraceData.o unpackSorter.o

ifdef GFAST_USE_EW
OBJS += unpackTraceBuf2Messages.o
endif
ifdef GFAST_ENABLE_GEOJSON
OBJS += unpackGeojsonMessages.o
endif

DEBUG	= -g

INCL            = -I ../../../include $(EW_INCL) $(HDF5_INCL) $(ISCL_INCL) $(JANSSON_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(cc) -c $< $(CFLAGS) $(DEBUG) $(INCL)

ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cc *.h
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
	-rm -f *.o $(BIN)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
