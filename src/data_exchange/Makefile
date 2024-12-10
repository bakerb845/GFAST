# Makefile for GFAST/src/data_exchange
#
GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= initializeDataConnection.o
LIB	= gfast_dataconn.a

DEBUG	= -g
#CCFLAGS	+= -std=c++0x -O0 -D_REENTRANT -Dstatic_config

INCL= -I ../../include $(INIPARSER_INCL) $(ISCL_INCL)

all: $(LIB)
lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(LIB) $(OBJS)

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

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
