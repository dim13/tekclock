# $Id$

CFLAGS+=	-Wall
PROG=		tekclock
SRCS=		tekclock.c tekplot.c
OBJS=		$(SRCS:.c=.o)
LDFLAGS+=		-lm

$(PROG): $(OBJS)

clean:
	@rm -f $(PROG) *.core core *.o
