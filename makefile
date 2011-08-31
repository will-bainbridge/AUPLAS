CC		= gcc
#CFLAGS		= -g -Wall -pg
CFLAGS		= -O2 -Wall

INCLS		= -I./thirdparty/UFconfig \
		  -I./thirdparty/CXSparse/Include \
		  -I./thirdparty/SuperLU_4.1/SRC \
		  -I./thirdparty/UMFPACK/Include \
		  -I./thirdparty/AMD/Include
LIBS		= -lm -lrt -lblas -llapack \
		  -L./thirdparty/CXSparse/Lib/ -lcxsparse \
		  -L./thirdparty/SuperLU_4.1/lib -lsuperlu_4.1 \
		  -L./thirdparty/UMFPACK/Lib -lumfpack \
		  -L./thirdparty/AMD/Lib -lamd

MAINSRCS	= preprocess.c solve.c
COMMONSRCS 	= io.c fetch.c geometry.c numerics.c memory.c system.c csr.c connectivity.c sort.c

SRCS		= $(MAINSRCS) $(COMMONSRCS)

MAINOBJS	= $(MAINSRCS:.c=.o)
COMMONOBJS	= $(COMMONSRCS:.c=.o)
OBJS		= $(MAINOBJS) $(COMMONOBJS)

EFILE		= $(MAINSRCS:.c=)

################################################################################

all: $(EFILE)

.SECONDEXPANSION:
$(EFILE): $$@.o $(COMMONOBJS)
	$(CC) $(CFLAGS) -o $@ $@.o $(COMMONOBJS) $(LIBS)

$(OBJS): makefile
	$(CC) $(CFLAGS) $(INCLS) -c $*.c

-include depend
depend: $(SRCS)
	$(CC) -MM $(INCLS) $^ > $@

clean:
	rm -f *o depend $(EFILE) *~
