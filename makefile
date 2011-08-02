CC		= gcc
CFLAGS		= -g
INCLS		= -I ~/include/
LIBS		= -L -lm

MAINSRCS	= preprocess.c
COMMONSRCS 	= allocate.c io.c connectivity.c
HEADS		= auplas.h allocate.h

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

$(OBJS): $(HEADS) makefile
	$(CC) $(CFLAGS) $(INCLS) -c $*.c

-include depend
depend: $(SRCS)
	$(CC) -MM $(INCLS) $^ > $@

clean:
	rm -f *o depend $(EFILE) *~
