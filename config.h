#ifndef _INCLUDED_TIMELIMIT_CONFIG_H
#define _INCLUDED_TIMELIMIT_CONFIG_H

/* $Id$ */

/* we hope all OS's have those..*/
#include <sys/types.h>
#include <sys/signal.h>

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#ifdef HAVE_ERR
#include <err.h>
#endif /* HAVE_ERR */

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#else
#define EX_OK		0	/* successful termination */
#define EX__BASE	64	/* base value for error messages */
#define EX_USAGE	64	/* command line usage error */
#define EX_DATAERR	65	/* data format error */
#define EX_NOINPUT	66	/* cannot open input */
#define EX_NOUSER	67	/* addressee unknown */
#define EX_NOHOST	68	/* host name unknown */
#define EX_UNAVAILABLE	69	/* service unavailable */
#define EX_SOFTWARE	70	/* internal software error */
#define EX_OSERR	71	/* system error (e.g., can't fork) */
#define EX_OSFILE	72	/* critical OS file missing */
#define EX_CANTCREAT	73	/* can't create (user) output file */
#define EX_IOERR	74	/* input/output error */
#define EX_TEMPFAIL	75	/* temp failure; user is invited to retry */
#define EX_PROTOCOL	76	/* remote error in protocol */
#define EX_NOPERM	77	/* permission denied */
#define EX_CONFIG	78	/* configuration error */
#define EX__MAX	78	/* maximum listed value */
#endif /* HAVE_SYSEXITS_H */

#ifndef __unused
#ifdef __GNUC__
#define __unused __attribute__((unused))
#else  /* __GNUC__ */
#define __unused
#endif /* __GNUC__ */
#endif /* __unused */

#endif /* _INCLUDED */
