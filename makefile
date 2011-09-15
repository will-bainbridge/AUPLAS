CC		= gcc
CFLAGS		= -O2 -Wall

HOME		= .
THIRDPARTY	= $(HOME)/thirdparty

#------------------------------------------------------------------------------#

INCLUDES	+= -I$(THIRDPARTY)/UMFPACK/Include -I$(THIRDPARTY)/AMD/Include -I$(THIRDPARTY)/UFconfig
LIBRARIES	+= -L$(THIRDPARTY)/UMFPACK/Lib -L$(THIRDPARTY)/AMD/Lib -lumfpack -lamd

INCLUDES	+= -I$(THIRDPARTY)/ilupackV2.4_GNU64_MUMPS/include -I$(THIRDPARTY)/AMD/Include
LIBRARIES	+= -Wl,--start-group -L$(THIRDPARTY)/ilupackV2.4_GNU64_MUMPS/lib/GNU64 -L$(THIRDPARTY)/AMD/Lib \
		   -lilupack -lmumps -lamd -lmetis -lsparspak -lblaslike -Wl,--end-group

#LIBRARIES	+= -llapack -lblas
#LIBRARIES	+= -Wl,--start-group -lpthread -L/opt/intel/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -Wl,--end-group -Wl,-R/opt/intel/mkl/lib/intel64
LIBRARIES	+= -L$(THIRDPARTY)/GotoBLAS2 -Wl,-R$(THIRDPARTY)/GotoBLAS2 -lgoto2

LIBRARIES	+= -lm -lrt -lgfortran

#------------------------------------------------------------------------------#

MAINSOURCES	= preprocess.c solve.c postprocess.c
COMMONSOURCES 	= io.c fetch.c geometry.c numerics.c memory.c system.c csr.c connectivity.c expression.c

SOURCES		= $(MAINSOURCES) $(COMMONSOURCES)

MAINOBJECTS	= $(MAINSOURCES:.c=.o)
COMMONOBJECTS	= $(COMMONSOURCES:.c=.o)
OBJECTS		= $(MAINOBJECTS) $(COMMONOBJECTS)

EXECUTABLES	= $(MAINSOURCES:.c=)

################################################################################

all: $(EXECUTABLES)

.SECONDEXPANSION:
$(EXECUTABLES): $$@.o $(COMMONOBJECTS)
	$(CC) $(CFLAGS) -o $@ $@.o $(COMMONOBJECTS) $(LIBRARIES)

$(OBJECTS): makefile
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

-include depend
depend: $(SOURCES)
	$(CC) -MM $(INCLUDES) $^ > $@

clean:
	rm -f *o depend $(EXECUTABLES) *~
