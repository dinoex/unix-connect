/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999-2000  Dirk Meyer
 *  Copyright (C) 1999-2000  Matthias Andree
 *  Copyright (C) 2000       Krischan Jodies
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
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
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
#include <sys/types.h>
#include "iwait.h"
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "istring.h"
#include <stdarg.h>
#include <utime.h>
#include <sys/stat.h>
#include <errno.h>

#include "utility.h"
#include "boxstat.h"
#include "ministat.h"
#include "xprog.h"
#include "uudebug.h"
#include "uulog.h"

#ifdef __GNUC__
__inline__
#endif
static void
freearglist(char **arg)
{
	for(;*arg;arg++) dfree(*arg);
}

/* eine Art system, aber auf execv-Basis und ohne shell
   Warnung: Pfad muss mit angegeben werden! */
static int runcommand(const char *file, ...)
{
#if defined (NEXTSTEP)
	union wait estat;
#else
	int estat;
#endif
	int ret;
	char *arg[20];
	const char *next;
	const char *start;
	char *line;
	char *work;
	char *new;
	va_list ap;
	pid_t c_pid;
	size_t i;

	logcwd("runcommand");

	i = 0;
	next = file;
	start = next;
	line = dstrdup( file );
	work = line;
	new = work;
	*work = 0;
	while ( *start ) {
		/* dow we have to escape? */
		if ( *next == '\\' ) {
			if ( *(++next) == 0 )
				break;
			*(work++) = *(next++);
			continue;
		}
		/* argument */
		if ( ( *next != ' ' )
		&& ( *next != '\t' )
		&& ( *next != 0 ) ) {
			*(work++) = *(next++);
			continue;
		}
		/* ignore leading whitespaces */
		if ( next == start ) {
			start ++;
			next ++;
			continue;
		}
		/* we do have data now */
		if (i >= sizeof(arg)/sizeof(arg[0])-1) {
			dfree( line );
			freearglist(arg);
			return -1;
		}
		/* this is the end of the argument */
		*(work++) = 0;
		if ( *next != 0 )
			next ++;
		arg[i++] = dstrdup( new );
		start = next;
		new = work;
	}
	va_start(ap, file);
	dfree( line );
	while (i < sizeof(arg)/sizeof(arg[0])-1) {
		work = va_arg(ap, char *);
		if ( work == NULL )
			break;
		arg[i++] = dstrdup( work );
	}
	va_end(ap);
	arg[i] = NULL;

	switch((c_pid = fork())) {
	case -1: /* parent, child cannot fork */
		newlog(ERRLOG, "runcommand: cannot fork for %s:%s",
			arg[0], strerror(errno));
		ret = -1;
		break;
	case 0: /* child */
		fprintf(stderr, "running %s ", arg[0]);
		{
			char * const *x;
			for (x = arg; *x; x++)
				fprintf(stderr, "%s ", *x);
			fprintf(stderr, "\n");
		}
		(void)execv(arg[0], (char * const *)arg);
		/* hier ist was schiefgelaufen, execv kehrt nicht zurueck */
		newlog(ERRLOG, "runcommand: execv failed for %s:%s",
			arg[0], strerror(errno));
		exit(-1);
	default: /* parent */
		wait(&estat);

		if (WIFEXITED(estat)) {
			ret = WEXITSTATUS(estat);
#ifdef DIRTY_ZMODEM_HACK
			if(ret == 128) {
				newlog(DEBUGLOG,
					"%s: return code %d hacked to 0",
					arg[0], ret);
				ret = 0;
			}
#endif
			newlog(ret ? ERRLOG : DEBUGLOG,
				"%s: returned %d\n",
				arg[0], ret);
			break;
		}
		if (WIFSIGNALED(estat)) {
			/* hier ist auch was schiefgelaufen, z. B.
			   das Kind hat SIGSEGV bekommen */
			newlog(ERRLOG,
				"%s: was terminated by signal %d\n",
				arg[0], WTERMSIG(estat));
			ret = -1;
			break;
		}
		newlog(ERRLOG,
			"did not finish properly, unhandled at %s:%d\n",
			__FILE__, __LINE__);
		ret = -1;
		break;
	}
	freearglist(arg);
	return ret;
}

static char *
findcmd(const char *proto, int upload)
     /* find protocol invokation according to proto and upload
	return: char * argname */
{
	char buffer[ 256 ];
	char *key;
	char *work;
	char *tail;
	FILE *fd;

	if ( xprogsfile == NULL ) {
		newlog(ERRLOG,
			"findcmd: mission configuration Xprogs-Konfig:" );
		return NULL;
	};

	if ( proto == NULL )
		return NULL;

	key = dalloc( strlen( proto + 20 ) );
	strcpy( key, proto );
	strcat( key, upload != 0 ? "-SEND" : "-RECV" );
	strupr( key );

	fd = fopen( xprogsfile, "rb" );
	if ( fd == NULL ) {
		newlog(ERRLOG, "findcmd: cannot open file %s:%s",
			xprogsfile, strerror(errno));
		dfree( key );
		return NULL;
	};

	*buffer = 0;
	while ( fgets( buffer, sizeof(buffer), fd ) != NULL ) {
		if ( *buffer == 0 )
			continue;
		if ( *buffer == '#' )
			continue;
		work = strchr( buffer, ',' );
		if ( work == NULL )
			continue;
		*(work++) = 0;
		strupr( buffer );
		if ( strstr( buffer, key ) == NULL )
			continue;
		while ( *work != 0 ) {
			if ( ( *work == ' ' )
			|| ( *work == '\t' ) ) {
				work ++;
				continue;
			};
			break;
		};
		if ( *work == '"' ) {
			work ++;
			tail = strrchr( work, '"' );
			if ( tail != NULL )
				*tail = 0;
		};
		return dstrdup( work );
	};
	newlog(ERRLOG, "findcmd: cannot find %s in file %s",
		key, xprogsfile );
	dfree( key );
	return NULL;
}


static int dofile(const char *proto, const char *file, int upload)
{
	char *cmd;
	char buf[1024];

	if ((cmd = findcmd (proto, upload))) {
		snprintf(buf, sizeof(buf), cmd, file);
		dfree( cmd );
		return runcommand(buf, NULL);
	} else {
		return -1;
	}
}

int recvfile(const char *proto, const char *file)
{
	int rc;

	rc = dofile(proto, file, 0);
	/* einige versiffte Muell-ZModems, namentlich das von
	   Martin Brueckner, schicken defektes Filedatum und defekte
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

	if(strlen(arcer) > sizeof(arg)) return -1;
	qstccpy(arg, arcer, sizeof(arg));
	strupr(arg);

	newlog(DEBUGLOG,"auspack %s %s", arg, arcfile);
	return runcommand(auspack, arg, arcfile, NULL);
}

int call_import(const char *sysname)
{
	return runcommand(import_prog, sysname, "ZCONNECT", NULL);
}

