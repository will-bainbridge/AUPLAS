CC		= gcc
CFLAGS		= -g -Wall
INCLS		= -I./thirdparty/CXSparse/Include -I./thirdparty/UFconfig -I./thirdparty/SuperLU_4.1/SRC
LIBS		= -lm -llapack -L./thirdparty/CXSparse/Lib/ -lcxsparse -L./thirdparty/SuperLU_4.1/lib -lsuperlu_4.1

MAINSRCS	= preprocess.c solve.c
COMMONSRCS 	= io.c connectivity.c fetch.c geometry.c numerics.c memory.c system.c csr.c divergence.c mesh.c

SRCS		= $(MAINSRCS) $(COMMONSRCS)

MAINOBJS	= $(MAINSRCS:.c=.o)
COMMONOBJS	= $(COMMONSRCS:.c=.o)
OBJS		= $(MAINOBJS) $(COMMONOBJS)

EFILE		= $(MAINSRCS:.c=)

################################################################################

all: $(EFILE)

.SECONDEXPANSION:
$(EFILE): $$@.o $(COMMONOBJS)
	$(CC) -o $@ $@.o $(COMMONOBJS) $(LIBS)

$(OBJS): makefile
	$(CC) $(CFLAGS) $(INCLS) -c $*.c

-include depend
depend: $(SRCS)
	$(CC) -MM $(INCLS) $^ > $@

clean:
	rm -f *o depend $(EFILE) *~
