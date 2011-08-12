CC		= gcc
CFLAGS		= -O2 -Wall
INCLS		= -I ~/include/
LIBS		= -lm -llapack

MAINSRCS	= preprocess.c solve.c
COMMONSRCS 	= io.c connectivity.c fetch.c geometry.c numerics.c memory.c

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
