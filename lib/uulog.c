/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1994     Peter Much
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
 *  Sat Jul  1 20:45:39 MET DST 1995 (P.Much)
 *  - Erweitert zur Nutzung der syslog-facility mit -DHAVE_SYSLOG.
 */


# include "config.h"
# include <stdio.h>
# include <stdarg.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif
# include <time.h>
# include "uulog.h"
# include "ministat.h"

extern char *logdir;


void
#ifdef HAVE_SYSLOG
logfile(int lchan,
#else
logfile(char *lfilename,
#endif
const char *from, const char *to, const char *mid, const char *format, ...)
{
	va_list ap;
#ifdef HAVE_SYSLOG
	int prio;
	char text[20];
#else
	FILE *f;
	time_t now;
	char buf[200];
#endif

	minireadstat();
#ifdef HAVE_SYSLOG
	switch(lchan) {
	    case Z2ULOG	:
		prio = Z2ULOG_PRIO;
		strcpy(text, Z2ULOG_NAME);
		break;;
	    case U2ZLOG	:
		prio = U2ZLOG_PRIO;
		strcpy(text, U2ZLOG_NAME);
		break;;
	    case ERRLOG	:
		prio = ERRLOG_PRIO;
		strcpy(text, ERRLOG_NAME);
		break;;
	    case INCOMING :
		prio = INCOMING_PRIO;
		strcpy(text, INCOMING_NAME);
		break;;
	    case OUTGOING :
		prio = OUTGOING_PRIO;
		strcpy(text, OUTGOING_NAME);
		break;;
	    case XTRACTLOG :
		prio = XTRACTLOG_PRIO;
		strcpy(text, XTRACTLOG_NAME);
		break;;
	    case DEBUGLOG :
		prio = DEBUGLOG_PRIO;
		strcpy(text, DEBUGLOG_NAME);
		break;;
	    default:
		prio = LOG_ERR;
		strcpy(text, "unknown");
	}
	openlog(SYSLOG_LOGNAME, LOG_PID, SYSLOG_KANAL);
	if(format == NULL || strlen(format) == 0)
		syslog(prio, "%s \"%s\"\t\"%s\"\t%s", text, from, to, mid);
	else {
		char tmpformat[9 + strlen(text) + strlen(from) + strlen (to) + strlen(mid) + strlen(format)];

		sprintf(tmpformat, "%s \"%s\"\t\"%s\"\t%s\t%s", text, from, to, mid, format);
		va_start(ap, format);
		vsyslog(prio, tmpformat, ap);
		va_end(ap);
	}
	closelog();
#else
	sprintf(buf, "%s/%s", logdir, lfilename);
	f = fopen(buf, "a");
	if (!f) {
		fprintf(stderr, "\nFATAL: Logfile \"%s\" nicht schreibbar\n", buf);
		return;
	}
	now = time(NULL);
	strftime(buf, 200, "%Y/%m/%d %H:%M:%S", localtime(&now));
	fprintf(f, "%s\t\"%s\"\t\"%s\"\t%s\t", buf, from, to, mid);
	va_start(ap, format);
	vfprintf(f, format, ap);
	va_end(ap);
	fclose(f);
#endif
}
