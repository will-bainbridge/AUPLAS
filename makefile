CC		= gcc
CFLAGS		= -g -Wall
INCLS		= -I ~/include/
LIBS		= -L -lm

MAINSRCS	= preprocess.c
COMMONSRCS 	= allocate.c io.c connectivity.c fetch.c
HEADS		= auplas.h allocate.h fetch.h

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
