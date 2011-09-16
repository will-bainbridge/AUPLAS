COMPILER	= gcc
FLAG		= -O2 -Wall

HOMEPATH	= $(shell pwd)
BUILDPATH	= $(HOMEPATH)/src
THIRDPATH	= $(HOMEPATH)/thirdparty

#------------------------------------------------------------------------------#

INCLUDE		+= -I$(THIRDPATH)/UMFPACK/Include -I$(THIRDPATH)/AMD/Include -I$(THIRDPATH)/UFconfig
LIBRARY		+= -L$(THIRDPATH)/UMFPACK/Lib -L$(THIRDPATH)/AMD/Lib -lumfpack -lamd

INCLUDE		+= -I$(THIRDPATH)/ilupackV2.4_GNU64_MUMPS/include -I$(THIRDPATH)/AMD/Include
LIBRARY		+= -Wl,--start-group -L$(THIRDPATH)/ilupackV2.4_GNU64_MUMPS/lib/GNU64 -L$(THIRDPATH)/AMD/Lib \
		   -lilupack -lmumps -lamd -lmetis -lsparspak -lblaslike -Wl,--end-group

#LIBRARY	+= -llapack -lblas
#LIBRARY	+= -Wl,--start-group -lpthread -L/opt/intel/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -Wl,--end-group -Wl,-R/opt/intel/mkl/lib/intel64
LIBRARY		+= -L$(THIRDPATH)/GotoBLAS2 -Wl,-R$(THIRDPATH)/GotoBLAS2 -lgoto2

LIBRARY		+= -lm -lrt -lgfortran

#------------------------------------------------------------------------------#

MAINFILES	= preprocess.c solve.c postprocess.c
COMMONFILES	= io.c fetch.c geometry.c numerics.c memory.c system.c csr.c connectivity.c expression.c

MAINSOURCE	= $(addprefix $(BUILDPATH)/,$(MAINFILES))
COMMONSOURCE 	= $(addprefix $(BUILDPATH)/,$(COMMONFILES))
ALLSOURCE	= $(MAINSOURCE) $(COMMONSOURCE)

MAINOBJECT	= $(MAINSOURCE:.c=.o)
COMMONOBJECT	= $(COMMONSOURCE:.c=.o)
ALLOBJECT	= $(MAINOBJECT) $(COMMONOBJECT)

EXECUTABLES	= $(MAINFILES:.c=)

################################################################################

all: $(EXECUTABLES)

.SECONDEXPANSION:
$(EXECUTABLES): $(BUILDPATH)/$$@.o $(COMMONOBJECT)
	$(COMPILER) $(FLAG) -o $@ $(BUILDPATH)/$@.o $(COMMONOBJECT) $(LIBRARY)

$(ALLOBJECT): makefile
	$(COMPILER) $(FLAG) -c $*.c -o $*.o $(INCLUDE)

-include $(BUILDPATH)/depend
depend: $(ALLSOURCE)
	$(COMPILER) -MM $(INCLUDE) $^ | sed 's|^\(.*\.o\)|$(BUILDPATH)/\1|g'  > $(BUILDPATH)/$@

clean:
	rm -f $(BUILDPATH)/*o
