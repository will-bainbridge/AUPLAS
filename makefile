CC		= gcc
CFLAGS		= -O2 -Wall
FC		= gfortran
FFLAGS		= -O2 -Wall

INCLS		= -I./thirdparty/UFconfig \
		  -I./thirdparty/UMFPACK/Include \
		  -I./thirdparty/AMD/Include \
		  -I./thirdparty/ilupackV2.4_GNU64_MUMPS/include

UMFPACKLIBS	= -L./thirdparty/UMFPACK/Lib -L./thirdparty/AMD/Lib -lumfpack -lamd
ILUPACKLIBS	= -L./thirdparty/ilupackV2.4_GNU64_MUMPS/lib/GNU64 -L./thirdparty/AMD/Lib -lilupack -lmumps -lamd -lmetis -lsparspak -lblaslike
#LINEARLIBS	= -llapack -lblas
LINEARLIBS	= -Wl,--start-group -lpthread -static -L/opt/intel/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -Wl,--end-group
LIBS		= $(UMFPACKLIBS) $(ILUPACKLIBS) $(LINEARLIBS) -lm -lrt

MAINSRCS	= preprocess.c solve.c postprocess.c
COMMONSRCS 	= io.c fetch.c geometry.c numerics.c memory.c system.c csr.c connectivity.c

SRCS		= $(MAINSRCS) $(COMMONSRCS)

MAINOBJS	= $(MAINSRCS:.c=.o)
COMMONOBJS	= $(COMMONSRCS:.c=.o)
OBJS		= $(MAINOBJS) $(COMMONOBJS)

EFILE		= $(MAINSRCS:.c=)

################################################################################

all: $(EFILE)

.SECONDEXPANSION:
$(EFILE): $$@.o $(COMMONOBJS)
	$(FC) $(FFLAGS) -o $@ $@.o $(COMMONOBJS) $(LIBS)

$(OBJS): makefile
	$(CC) $(CFLAGS) $(INCLS) -c $*.c

-include depend
depend: $(SRCS)
	$(CC) -MM $(INCLS) $^ > $@

clean:
	rm -f *o depend $(EFILE) *~
