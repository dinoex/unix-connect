/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
 *  Copyright (C) 1999     Dirk Meyer
 *  Copyright (C) 1999     Matthias Andree
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------
 * Eine deutsche Zusammenfassung zum Copyright dieser Programme sowie der
 * GNU General Public License finden Sie in der Datei README.1st.
 * ------------------------------------------------------------------------
 *
 *  Bugreports, suggestions for improvement, patches, ports to other systems
 *  etc. are welcome. Contact the maintainer by e-mail:
 *  dirk.meyer@dinoex.sub.org or snail-mail:
 *  Dirk Meyer, Im Grund 4, 34317 Habichstwald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif
#include <stdarg.h>
#include <utime.h>
#include <sys/stat.h>

#include "utility.h"
#include "boxstat.h"
#include "ministat.h"
#include "xprog.h"
#include "uudebug.h"

/* eine Art system, aber auf execv-Basis und ohne shell
   Warnung: Pfad muss mit angegeben werden! */
static int runcommand(const char *file, ...)
{
	const char *arg[20];
	va_list ap;
	pid_t c_pid;
	size_t i;

	logcwd("runcommand");
	i = 0;
	arg[i++] = file;
	va_start(ap, file);
	while((i < sizeof(arg))
	      && (arg[i] = va_arg(ap, char *))) {
		i++;
	}
	va_end(ap);

	switch((c_pid = fork())) {
	case -1: /* parent, child cannot fork */
		perror(file);
		return -1;
	case 0: /* child */
		fprintf(stderr, "running %s ", file);
		{
			const char * const *x;
			for (x = arg; *x; x++)
				fprintf(stderr, "%s ", *x);
			fprintf(stderr, "\n");
		}
#ifdef linux
		(void)execv(file, arg);
#else
		(void)execv(file, (char * const *)arg);
#endif
		/* hier ist was schiefgelaufen, execv kehrt nicht zurueck */
		perror(file);
		exit (-1);
	default: /* parent */
		{
#if defined (NEXTSTEP)
			union wait stat;
#else
			int stat;
#endif
			wait(&stat);

			if (WIFEXITED(stat)) {
#ifdef DIRTY_ZMODEM_HACK
				if (WEXITSTATUS(stat)==128)
					return 0;
#endif
				fprintf(stderr, "returned %d\n",
					WEXITSTATUS(stat));
				return WEXITSTATUS(stat);
			} else {
				/* hier ist auch was schiefgelaufen, z. B.
				   das Kind hat SIGSEGV bekommen */
				fprintf(stderr, "did not finish properly\n");
				return 1;
			}
		}
		break;
	}

}

static struct {
	const char *proto;
	int direction; /* 0 = download, != 0 = upload */
	const char *cmd;
	int needfile;
} transports[] = {
	{ "ZMODEM", 0, "/usr/bin/rz", 0 },
	{ "ZMODEM", 1, "/usr/bin/sz", 1 },
	{ "YMODEM", 0, "/usr/bin/rb", 0 },
	{ "YMODEM", 1, "/usr/bin/sb", 1 },
	{ "XMODEM", 0, "/usr/bin/rx", 1 },
 	{ "XMODEM", 1, "/usr/bin/sz -X", 1 },
	{ 0,0,0,0 }
};

static const char *findcmd(const char *proto, int upload, int *needfile)
     /* find protocol invokation according to proto and upload */
     /* output: *needfile -> if true, protocol needs filename
	appended
	return: char * argname */
{
	int i = 0;
	while (transports[i].proto) {
		if(upload == transports[i].direction
		   && !stricmp(transports[i].proto, proto)) {
			*needfile = transports[i].needfile;
			return transports[i].cmd;
		}
		i++;
	}
	return NULL;
}


static int dofile(const char *proto, const char *file, int upload)
{
	const char *cmd;
	char buf[1024];
	int needsfile;

	if ((cmd = findcmd (proto, upload, &needsfile))) {
#ifdef HAVE_SNPRINTF
		snprintf(buf, sizeof(buf), cmd, file);
#else
#warning "sprintf is insecure"
		sprintf(buf, cmd, file);
#endif
		if(needsfile)
			return runcommand(cmd, "-vv", file, NULL);
		else
			return runcommand(cmd, "-vv", NULL);
	} else {
		return -1;
	}
}

int recvfile(const char *proto, const char *file)
{
	int rc;

	rc = dofile(proto, file, 0);
	/* einige versiffte Müll-ZModems, namentlich das von
	   Martin Brückner, schicken defektes Filedatum und defekte
	   Permissions. Das wird hier gefixt. */
	if (utime(file, NULL))
		perror(file); /* korrekte Zeit erzeugen */
	if (chmod(file, S_IRUSR|S_IWUSR|S_IRGRP))
		perror(file); /* korrekte Modes setzen */
	return rc;
}

int sendfile(const char *proto, const char *file)
{
	return dofile(proto, file, 1);
}

int call_auspack(const char *arcer, const char *arcfile)
{
	char arg[1024];

	stccpy(arg, arcer, sizeof(arg));
	arg[sizeof(arg-1)] = '\0';
	strupr(arg);

	return runcommand(auspack, arg, arcfile, NULL);
}


int call_import(const char *sysname)
{
	return runcommand(import_prog, sysname, "ZCONNECT", NULL);
}

