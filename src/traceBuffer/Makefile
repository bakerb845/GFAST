# Makefile for GFAST/src/traceBuffer

GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

DEBUG	= -g

INCL            = -I ../../include $(HDF5_INCL) $(EW_INCL) $(CBLAS_INCL) $(ISCL_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

SDIRS	= gnsstrace h5
LIB	= traceBuffer.a
OBJS := $(foreach dir,$(SDIRS),$(dir)/*.o) 

all: $(LIB)
lib: $(LIB)
# %.o:    %.c
# 	$(cc) -c $< $(CFLAGS) $(DEBUG) $(INCL)

# ids:    *.cc *.h
#         $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

# rm-ids:    *.cc *.h
#         $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

$(LIB): $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir all ; \
	done
	$(AR) $(LIB) $(OBJS)

ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(SDIRS) ; do \
		make -C $$dir install ; \
	done

docs:
	doxygen doxygen.conf

cleandocs:
	rm -rf ../docs/epic

clean:
	-rm -f *.o $(LIB)
	for dir in $(SDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean: cleandocs
	for dir in $(SDIRS) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(SDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
