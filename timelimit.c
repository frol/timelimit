/*-
 * Copyright (c) 2001, 2007  Peter Pentchev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#define PARSE_CMDLINE

unsigned long	warntime, killtime;
unsigned long	warnsig, killsig;
volatile int	fdone, falarm, fsig, sigcaught;
int		quiet;

static const char cvs_id[] =
"$Ringlet$";

static struct {
	const char	*name, opt;
	unsigned long	*var;
} envopts[] = {
	{"KILLSIG",	'S',	&killsig},
	{"KILLTIME",	'T',	&killtime},
	{"WARNSIG",	's',	&warnsig},
	{"WARNTIME",	't',	&warntime},
	{NULL,		0,	NULL}
};

#ifndef HAVE_ERR
static void	err(int, const char *, ...);
static void	errx(int, const char *, ...);
#endif /* !HAVE_ERR */

static void	usage(void);

static void	init(int, char *[]);
static void	doit(char *[]);
static void	child(char *[]);
static void	terminated(const char *);

#ifndef HAVE_ERR
static void
err(int code, const char *fmt, ...) {
	va_list v;

	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);

	fprintf(stderr, ": %s\n", strerror(errno));
	exit(code);
}

static void
errx(int code, const char *fmt, ...) {
	va_list v;

	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);

	fprintf(stderr, "\n");
	exit(code);
}

static void
warn(const char *fmt, ...) {
	va_list v;

	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);

	fprintf(stderr, ": %s\n", strerror(errno));
}

static void
warnx(const char *fmt, ...) {
	va_list v;

	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);

	fprintf(stderr, "\n");
}
#endif /* !HAVE_ERR */

static void
usage(void) {
	errx(EX_USAGE, "usage: timelimit [-q] [-S ksig] [-s wsig] "
	    "[-T ktime] [-t wtime] command");
}

static unsigned long
atou_fatal(const char *s) {
	unsigned long v;
	const char *p;

	v = 0;
	for (p = s; (*p >= '0') && (*p <= '9'); p++)
		v = v * 10 + *p - '0';
	if (*p != '\0')
		usage();
	return (v);
}

static void
init(int argc, char *argv[]) {
#ifdef PARSE_CMDLINE
	int ch;
#endif
	int optset;
	unsigned i;
	char *s;
	
	/* defaults */
	quiet = 0;
	warnsig = SIGTERM;
	killsig = SIGKILL;
	warntime = 3600;
	killtime = 120;

	optset = 0;
	
	/* process environment variables first */
	for (i = 0; envopts[i].name != NULL; i++)
		if ((s = getenv(envopts[i].name)) != NULL) {
			*envopts[i].var = atou_fatal(s);
			optset = 1;
		}

#ifdef PARSE_CMDLINE
	while ((ch = getopt(argc, argv, "qS:s:T:t:")) != EOF) {
		switch (ch) {
			case 'q':
				quiet = 1;
				break;
			default:
				/* check if it's a recognized option */
				for (i = 0; envopts[i].name != NULL; i++)
					if (ch == envopts[i].opt) {
						*envopts[i].var =
						    atou_fatal(optarg);
						optset = 1;
						break;
					}
				if (envopts[i].name == NULL)
					usage();
		}
	}
#else
	optind = 1;
#endif

	if (!optset) /* && !quiet? */
		warnx("using defaults: warntime=%lu, warnsig=%lu, "
		    "killtime=%lu, killsig=%lu",
		    warntime, warnsig, killtime, killsig);

	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();

	/* sanity checks */
	if ((warntime == 0) || (killtime == 0))
		usage();
}

static void
sigchld(int sig __unused) {

	fdone = 1;
}

static void
sigalrm(int sig __unused) {

	falarm = 1;
}

static void
sighandler(int sig) {

	sigcaught = sig;
	fsig = 1;
}

static void
setsig_fatal(int sig, void (*handler)(int)) {
#ifdef HAVE_SIGACTION
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;
	act.sa_flags = 0;
#ifdef SA_NOCLDSTOP
	act.sa_flags |= SA_NOCLDSTOP;
#endif /* SA_NOCLDSTOP */
	if (sigaction(sig, &act, NULL) < 0)
		err(EX_OSERR, "setting signal handler for %d", sig);
#else  /* HAVE_SIGACTION */
	if (signal(sig, handler) == SIG_ERR)
		err(EX_OSERR, "setting signal handler for %d", sig);
#endif /* HAVE_SIGACTION */
}
    
static void
doit(char *argv[]) {
	pid_t pid;

	/* install signal handlers */
	fdone = falarm = fsig = sigcaught = 0;
	setsig_fatal(SIGALRM, sigalrm);
	setsig_fatal(SIGCHLD, sigchld);
	setsig_fatal(SIGTERM, sighandler);
	setsig_fatal(SIGHUP, sighandler);
	setsig_fatal(SIGINT, sighandler);
	setsig_fatal(SIGQUIT, sighandler);

	/* fork off the child process */
	if ((pid = fork()) < 0)
		err(EX_OSERR, "fork");
	if (pid == 0)
		child(argv);

	/* sleep for the allowed time */
	alarm(warntime);
	while (!(fdone || falarm || fsig))
		pause();
	alarm(0);

	/* send the warning signal */
	if (fdone)
		return;
	if (fsig)
		terminated("run");
	falarm = 0;
	if (!quiet)
		warnx("sending warning signal %lu", warnsig);
	kill(pid, (int) warnsig);

#ifndef HAVE_SIGACTION
	/* reset our signal handlers, just in case */
	setsig_fatal(SIGALRM, sigalrm);
	setsig_fatal(SIGCHLD, sigchld);
	setsig_fatal(SIGTERM, sighandler);
	setsig_fatal(SIGHUP, sighandler);
	setsig_fatal(SIGINT, sighandler);
	setsig_fatal(SIGQUIT, sighandler);
#endif /* HAVE_SIGACTION */

	/* sleep for the grace time */
	alarm(killtime);
	while (!(fdone || falarm || fsig))
		pause();
	alarm(0);

	/* send the kill signal */
	if (fdone)
		return;
	if (fsig)
		terminated("grace");
	if (!quiet)
		warnx("sending kill signal %lu", killsig);
	kill(pid, (int) killsig);
}

static void
terminated(const char *period) {

	errx(EX_SOFTWARE, "terminated by signal %d during the %s period",
	    sigcaught, period);
}

static void
child(char *argv[]) {

	execvp(argv[0], argv);
	err(EX_OSERR, "executing %s", argv[0]);
}

int
main(int argc, char *argv[]) {

	init(argc, argv);
	argc -= optind;
	argv += optind;
	doit(argv);
	return (EX_OK);
}
