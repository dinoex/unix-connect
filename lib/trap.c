/*
 * trap.c
 * $Id$
 *
 * Funktionen fuer Signal-Behandlung fuer unix-connect
 * fuer den Fall eines Programmfehlers, um Datenverlusten
 * vorzubeugen
 *
 * init_trap() installiert Handler fuer Signale, die
 *             zu core fuehren
 *
 * trap_handler() ist der Handler, der eine Nachricht
 *	       ins Logfile und nach stderr sowie
 *	       den core in das logfile-Verzeichnis
 *	       schreibt
 *
 * 9. Sep 1996 - Linux-faehige Version von Erich Oldendorf <olden@komtron.com>
 *
 * BUGS:
 *
 * Es schreibt noch nicht in ein Logfile.
 * syslog sollte benutzt werden.
 */

#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>

#include "trap.h"
#include "ministat.h"

typedef void (*sighandler_t)(int);
sighandler_t s_handler(int sig, sighandler_t addr);

static signal_struct signallist[] = {
	{	SIGQUIT,	"SIGQUIT"	},
	{	SIGILL,		"SIGILL"	},
	{	SIGTRAP,	"SIGTRAP"	},
	{	SIGABRT,	"SIGABRT"	},
/*	{	SIGEMT,		"SIGEMT"	},	*/
	{	SIGFPE,		"SIGFPE"	},
	{	SIGBUS,		"SIGBUS"	},
	{	SIGSEGV,	"SIGSEGV"	},
/*	{	SIGSYS,		"SIGSYS"	},	*/
/*	{	SIGLOST,	"SIGLOST"	},	*/
	{	0,		"???"		}
};
static char *me;

void init_trap(char *progname) {
	signal_struct *sp;

	me = progname;
	for (sp = signallist; sp->no; sp++)
#ifdef BSD
		signal(sp->no, (sighandler_t)s_handler);
#else
	/* sighandler_t und __sighandler_t sind dasselbe,
	   aber das kapiert der gcc nicht und meckert. */
		signal(sp->no, (__sighandler_t)s_handler);
#endif
}

sighandler_t s_handler(int sig, sighandler_t addr)
{
	signal_struct *sp;

	for (sp=signallist; sp->no && sp->no != sig; sp++);

	fprintf(stderr, "%s: caught signal %s\n", me, sp->name);
	fprintf(stderr, "%s: dumping core to %s\n", me, logdir);
	chdir(logdir);
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
	return 0; /* not reached, just for the compiler... */
}
