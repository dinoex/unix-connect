/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 1994       Peter Much
 *  Copyright (C) 1999       Andreas Barth
 *  Copyright (C) 1999-2000  Dirk Meyer
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


/*
 *  uulog.c
 *
 *  Logfile-Routinen fuer den ZCONNECT/RFC GateWay
 *
 */


#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif
#include <time.h>

#include "uulog.h"

#define	MAX_LOG_LINE	200

#ifndef HAVE_SYSLOG
static char name[30];
FILE *uudeblogfile;
#endif

void
initlog(const char *xname) {
#ifdef HAVE_SYSLOG
	openlog(xname, LOG_PID|LOG_CONS, SYSLOG_KANAL);
#else
	stccpy(name,xname,sizeof(name));
	uudeblogfile = NULL;
#endif
}

void
newlog(int lchan, const char *format, ...)
{
	int prio;
	char buf[ MAX_LOG_LINE ];
#ifndef HAVE_SYSLOG
	FILE *logf;
	time_t now;
	char filename[20];
	char buf1[ MAX_LOG_LINE ];
#endif
	const char *defformat = "%s\n";
	va_list arg;

	switch(lchan) {
	case Z2ULOG :
		prio = Z2ULOG_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, Z2ULOG_FILE, sizeof(filename));
#endif
		break;
	case U2ZLOG :
		prio = U2ZLOG_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, U2ZLOG_FILE, sizeof(filename));
#endif
		break;
	case ERRLOG :
		prio = ERRLOG_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, ERRLOG_FILE, sizeof(filename));
#endif
		break;
	case INCOMING :
		prio = INCOMING_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, INCOMING_FILE, sizeof(filename));
#endif
		break;
	case OUTGOING :
		prio = OUTGOING_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, OUTGOING_FILE, sizeof(filename));
#endif
		break;
	case XTRACTLOG :
		prio = XTRACTLOG_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, XTRACTLOG_FILE, sizeof(filename));
#endif
		break;
	case DEBUGLOG :
		prio = DEBUGLOG_PRIO;
#ifndef HAVE_SYSLOG
		stccpy(filename, DEBUGLOG_FILE, sizeof(filename));
#endif
		break;
	default:
		prio = LOG_ERR;
#ifndef HAVE_SYSLOG
		stccpy(filename, DEBUGLOG_FILE, sizeof(filename));
#endif
		lchan = DEBUGLOG;
		break;
	}

	if(format == NULL || strlen(format) == 0)
		format=defformat;

	va_start (arg, format);
	vsnprintf (buf, MAX_LOG_LINE, format, arg);
	va_end (arg);

#ifdef HAVE_SYSLOG
	switch(lchan) {
	case Z2ULOG :
	case U2ZLOG :
		/* keine Ausgabe auf stderr fuer uursmtp/uuwsmtp */
		break;
	case ERRLOG :
	case INCOMING :
	case OUTGOING :
	case XTRACTLOG :
	case DEBUGLOG :
	default:
		fprintf(stderr, "%s\n", buf);
		break;
	}
	syslog(prio, "%s", buf);
#else
	if( lchan == DEBUGLOG ) {
		if ( uudeblogfile == NULL ) {
			uudeblogfile = fopen("/tmp/zlog", "a");
		}
		if ( uudeblogfile == NULL ) {
			fprintf(stderr,
			"\nFehler: Debug-Logfile nicht schreibbar\n" );
			uudeblogfile = stderr;
		}
		logf = uudeblogfile;
	} else {
		snprintf(buf,  sizeof(buf), "%s/%s", logdir, lfilename);
		logf = fopen(buf, "a");
		if (!logf) {
			fprintf(stderr,
			"\nFehler: Logfile \"%s\" nicht schreibbar\n", buf);
			logf = stderr;
		}
	}
	now = time(NULL);
	strftime(buf1,  MAX_LOG_LINE , "%Y/%m/%d %H:%M:%S", localtime(&now));
	fprintf (logf, "%s: %s%s\n", name, buf1, buf);
	if( lchan != DEBUGLOG )
		fclose(logf);
#endif
}

