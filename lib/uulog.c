/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1994     Peter Much
 *  Copyright (C) 1999     Andreas Barth
 *  Copyright (C) 1999     Dirk Meyer
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


/*
 *  uulog.c
 *
 *  Logfile-Routinen für den ZCONNECT/RFC GateWay
 *
 */


#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif
#include <time.h>

#include "uulog.h"
#include "ministat.h"

#define	MAX_LOG_LINE	200

extern char *logdir;

#ifndef HAVE_SYSLOG
static char name[30];
FILE *uudeblogfile;
#endif

const char *def_debuglog;

void initlog(const char *xname) {
#ifdef HAVE_SYSLOG
	openlog(xname, LOG_PID, SYSLOG_KANAL);
#else
	strncpy(name,xname,30);
	name[29] = 0;
	uudeblogfile = NULL;
#endif
	minireadstat();
	return 0;
}

void
newlog(int lchan, const char *format, ...)
{
	int prio;
	char text[20];
	char filename[20];
	char buf[ MAX_LOG_LINE ];
#ifndef HAVE_SYSLOG
	FILE *logf;
	time_t now;
	char buf1[ MAX_LOG_LINE ];
#endif
	const char *defformat = "%s\n";
	va_list arg;

	switch(lchan) {
	    case Z2ULOG	:
		prio = Z2ULOG_PRIO;
		strcpy(text, Z2ULOG_NAME);
		strcpy(filename, Z2ULOG_FILE);
		break;
	    case U2ZLOG	:
		prio = U2ZLOG_PRIO;
		strcpy(text, U2ZLOG_NAME);
		strcpy(filename, U2ZLOG_FILE);
		break;
	    case ERRLOG	:
		prio = ERRLOG_PRIO;
		strcpy(text, ERRLOG_NAME);
		strcpy(filename, ERRLOG_FILE);
		break;
	    case INCOMING :
		prio = INCOMING_PRIO;
		strcpy(text, INCOMING_NAME);
		strcpy(filename, INCOMING_FILE);
		break;
	    case OUTGOING :
		prio = OUTGOING_PRIO;
		strcpy(text, OUTGOING_NAME);
		strcpy(filename, OUTGOING_FILE);
		break;
	    case XTRACTLOG :
		prio = XTRACTLOG_PRIO;
		strcpy(text, XTRACTLOG_NAME);
		strcpy(filename, XTRACTLOG_FILE);
		break;
	    case DEBUGLOG :
		prio = DEBUGLOG_PRIO;
		strcpy(text, DEBUGLOG_NAME);
		strcpy(filename, DEBUGLOG_FILE);
		break;
	    default:
		prio = LOG_ERR;
		strcpy(filename, DEBUGLOG_NAME);
		strcpy(filename, DEBUGLOG_FILE);
		lchan = DEBUGLOG;
	}

	if(format == NULL || strlen(format) == 0)
		format=defformat;

	va_start (arg, format);
	vsnprintf (buf, MAX_LOG_LINE, format, arg);
	va_end (arg);

#ifdef HAVE_SYSLOG
	syslog(prio, buf);
#else
	if( lchan == DEBUGLOG ) {
		if ( uudeblogfile == NULL ) {
			uuuudeblogfile = fopen("/tmp/zlog", "a");
		}
		if ( uudeblogfile == NULL ) {
			fprintf(stderr,
			"\nFehler: Debug-Logfile nicht schreibbar\n" );
			uudeblogfile = stderr;
		}
		logf = uudeblogfile;
	} else {
		sprintf(buf, "%s/%s", logdir, lfilename);
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

