# Makefile for GFAST
EEWDIR = ../..
GFAST_DIR = ..
# Make.include.gfast will include $(EEWDIR)/Make.include.$(shell uname)
include $(GFAST_DIR)/Make.include.gfast

EW_INCL	= -I$(EWDIR)/include
EWLIB	= $(EWDIR)/lib

DM_DIR		= $(EEWDIR)/libs/dmlib
DM_LIB		= $(DM_DIR)/libdm.a

# Used to add more information on program startup output
UFLAGS = -DBUILDER=\"$(BUILDER)\"

SUBDIRS     = activeMQ hdf5 traceBuffer core eewUtils xml dmlib
INCL    = -I ../include $(ISCL_INCL) $(ACTIVEMQ_INCL) $(EW_INCL) $(HDF5_INCL)
EXE     = gfast_eew
#gfast_playback
EWOBJS = $(EWLIB)/swap.o
OBJS	= activeMQ/gfast_amq.a eewUtils/gfast_utils.a xml/gfast_xml.a core/gfast_core.a \
traceBuffer/traceBuffer.a hdf5/hdf5.a activeMQ/gfast_amq.a dmlib/dmlibWrapper.cpp.o \
$(EWOBJS)

LIBS    =  $(EW_LIB) $(ISCL_LIB) $(INIPARSER_LIB) $(XML2_LIB) $(HDF5_LIB) $(COMPEARTH_LIB) \
$(ACTIVEMQ_LIB) $(DM_LIB) $(APR_LIB) $(LAPACKE_LIB) $(CBLAS_LIB) $(FFTW_LIB) $(SSL_LIB) \
$(XERCES_LIB) $(QLIB2_LIB) $(CRYPTO_LIB) $(EWLIB)/libew_mt.a

SYSLIBS = -ldl -lm -lrt -pthread -lz -lsz -lstdc++

%.o:    %.c
	$(cc) -c $< -o $@ $(UFLAGS) $(CFLAGS) $(DEBUG) $(INCL)

all: $(SUBDIRS) $(EXE)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: $(SUBDIRS)

gfast_playback: gfast_playback.o $(OBJS)
	$(CC) -o $@ $(CCFLAGS) gfast_eew.o $(UFLAGS) $(OBJS) $(LIBS) $(SYSLIBS)

gfast_eew: gfast_eew.o $(OBJS)
	$(CC) -o $@ $(CCFLAGS) gfast_eew.o $(UFLAGS) $(OBJS) $(LIBS) $(SYSLIBS)

activeMQ/gfast_amq.a:
	make -C activeMQ lib

eewUtils/gfast_utils.a:
	make -C eewUtils lib

xml/gfast_xml.a:
	make -C xml lib

core/gfast_core.a:
	make -C core lib

all: $(EXE)

ids: $(ALL)
	for dir in $(SUBDIRS) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(ALL)
	for dir in $(SUBDIRS) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir install ; \
	done

TESTDIR = ./tests
ut: 
	make -C $(TESTDIR) ut
test: 
	make -C $(TESTDIR) test

clean:
	-rm -f *.o $(EXE)
	for dir in $(SUBDIRS) ; do \
		make -C $$dir clean ; \
	done
	make -C $(TESTDIR) clean

veryclean: cleandocs cleancoverage
	for dir in $(SUBDIRS) ; do \
		make -C $$dir veryclean ; \
	done
	make -C $(TESTDIR) veryclean

depend:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir depend ; \
	done

# coverage
INFODIR = $(EEWDIR)/coverage/info
COVDIR = $(EEWDIR)/coverage/html

covdirs:
	if [ ! -d "$(COVDIR)" ]; then mkdir -p $(COVDIR); fi
	if [ ! -d "$(INFODIR)" ]; then mkdir -p $(INFODIR); fi

coverage: cleancoverage covdirs buildcoverage runcoverage
	genhtml $(INFODIR)/*.info -o $(COVDIR) -s --legend

cleancoverage: 
	-rm -rf *.gcno *.gcda *.gcov *.info $(COVDIR)
	make -C $(TESTDIR) cleancoverage
	for dir in $(SUBDIRS) ; do \
		make -C $$dir cleancoverage ; \
	done

buildcoverage: 
	for dir in $(SUBDIRS) ; do \
		make -C $$dir buildcoverage ; \
	done

runcoverage: 
	make -C tests runcoverage

# documentation
DOCDIR=$(EEWDIR)/docs/gfast
docs: doxygen.conf
	mkdir -p $(DOCDIR)
	doxygen doxygen.conf

cleandocs: 
	rm -rf $(DOCDIR) *.tag

printvars:
	@echo INIPARSER=$(INIPARSER)
	@echo INIPARSER_INCL=$(INIPARSER_INCL)
	@echo INIPARSER_LIB=$(INIPARSER_LIB)
