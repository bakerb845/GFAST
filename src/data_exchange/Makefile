# Makefile for GFAST/src/data_exchange
#
GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

DEBUG	= -g

INCL            = -I ../../include $(INIPARSER_INCL) $(ISCL_INCL) $(EW_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

SDIRS	= nats earthworm
LIB	= gfast_dataexchange.a
OBJS = dataExchange.o
SUBOBJS = $(foreach dir,$(SDIRS),$(dir)/*.o) 

all: $(LIB)
lib: $(LIB)

$(LIB): $(SDIRS) $(OBJS)
	for dir in $(SDIRS) ; do make -C $$dir all; done
	$(AR) $(LIB) $(OBJS) $(SUBOBJS)

%.o:    %.c
	$(cc) -c $< -o $@ $(CFLAGS) $(DEBUG) $(INCL)

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
	-rm -f *.o $(LIB)
	for dir in $(SDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/

printvars:
	@echo OBJS=$(OBJS)
