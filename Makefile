# $Id: Makefile,v 1.4 2001/05/16 21:38:48 roam Exp $

#CC=		gcc
#CFLAGS=		-O -pipe
#LFLAGS=

RM=		rm -f

PROG=		timelimit
OBJS=		timelimit.o
SRCS=		timelimit.c

# comment this if you do not have err(3) and errx(3) (most BSD systems do)
CFLAGS+=	-DHAVE_ERR
# comment this if you do not have the sysexits.h header file (most systems do)
CFLAGS+=	-DHAVE_SYSEXITS_H
# comment this if you do not have the errno.h header file (most systems do)
CFLAGS+=	-DHAVE_ERRNO_H
# comment this if you do not have sigaction(2) (most systems do)
CFLAGS+=	-DHAVE_SIGACTION

# development/debugging flags, you may safely ignore them
#CFLAGS+=	${BDECFLAGS}
#CFLAGS+=	-ggdb -g3

all:		${PROG}

clean:
		${RM} ${PROG} ${OBJS}

${PROG}:	${OBJS}
		${CC} ${LFLAGS} -o ${PROG} ${OBJS}

timelimit.o:	timelimit.c config.h
		${CC} ${CFLAGS} -c timelimit.c
