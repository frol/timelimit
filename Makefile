# $Ringlet: c/sys/timelimit/Makefile,v 1.6 2004/12/01 15:08:56 roam Exp $

CC?=		gcc
CFLAGS?=		-O -pipe
LDFLAGS?=
LFLAGS?=	${LDFLAGS}
LIBS?=

RM=		rm -f

PROG=		timelimit
OBJS=		timelimit.o
SRCS=		timelimit.c

MAN8=		timelimit.8
MAN8GZ=		${MAN8}.gz

LOCALBASE?=	/usr/local
PREFIX?=	${LOCALBASE}
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/man

BINOWN?=	root
BINGRP?=	wheel
BINMODE?=	555

MANOWN?=	root
MANGRP?=	wheel
MANMODE?=	644

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

all:		${PROG} ${MAN8GZ}

clean:
		${RM} ${PROG} ${OBJS} ${MAN8GZ}

${PROG}:	${OBJS}
		${CC} ${LFLAGS} -o ${PROG} ${OBJS}

timelimit.o:	timelimit.c config.h
		${CC} ${CFLAGS} -c timelimit.c

${MAN8GZ}:	${MAN8}
		gzip -c ${MAN8} > ${MAN8GZ}.tmp
		mv ${MAN8GZ}.tmp ${MAN8GZ}

install:	all
		install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} ${PROG} ${DESTDIR}${BINDIR}/
		install -c -o ${MANOWN} -g ${MANGRP} -m ${MANMODE} ${MAN8GZ} ${DESTDIR}${MANDIR}8/
