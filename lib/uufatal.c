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
 *  UUfatal.c
 *
 *  Logfile-Routinen fuer den ZCONNECT/RFC GateWay
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 *
 *  Sat Jul  1 20:45:39 MET DST 1995 (P.Much)
 *  - Erweitert zur Nutzung der syslog-facility mit -DHAVE_SYSLOG.
 *  - Fehlende Datumsausgabe ergaenzt.
 */

# include "config.h"
# include <stdio.h>
# include <stdarg.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif
# include <time.h>
#include <sysexits.h>

# include "ministat.h"
# include "uulog.h"

const char *nomem = "Nicht genug Hauptspeicher!";

/*@@
 *
 * NAME
 *   uufatal -- Fatalen Fehler loggen.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <uulog.h>
 *   void uufatal(char *prog, char *format, ...);
 *
 * DESCRIPTION
 *   Die Funktion loggt den aufgetretenen Fehler mit Datum und beendet
 *   das Programm.
 *
 * PARAMETER
 *   char *prog -- Name des verstorbenen Programms.
 *
 * RESULT
 *   Die Funktion kehrt nie zurueck.
 *
 * BUGS
 *
 */


void uufatal(const char *prog, const char *format, ...)
{
#ifndef HAVE_SYSLOG
	FILE *f;
	time_t now;
	char buf[200];
#endif
	va_list ap;

	minireadstat();
#ifdef HAVE_SYSLOG
	openlog(SYSLOG_LOGNAME, LOG_PID|LOG_CONS, SYSLOG_KANAL);
	if(format == NULL || strlen(format) == 0)
		syslog(FATALLOG_PRIO, "%s: (unknown)", prog);
	else {
		char tmpformat[3 + strlen(prog) + strlen(format)];

		sprintf(tmpformat, "%s: %s", prog, format);
		va_start(ap, format);
		vsyslog(FATALLOG_PRIO, tmpformat, ap);
		va_end(ap);
	}
	closelog();
#else
	sprintf(buf, "%s/" ERRLOG, logdir);
	f = fopen(buf, "a");
	if (!f) {
		fprintf(stderr,
			"\nFATAL: Logfile \""ERRLOG"\" nicht schreibbar\n");
		exit( EX_CANTCREAT );
	}
	now = time(NULL);
	strftime(buf, 200, "%Y/%m/%d %H:%M:%S", localtime(&now));
	fprintf(f, "%s\t%s:\t", buf, prog);
	va_start(ap, format);
	vfprintf(f, format, ap);
	va_end(ap);
	fputs("\n", f);
	fclose(f);
#endif
	fprintf(stderr, "\n\n%s:\t", prog);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputs("\n", stderr);

	exit( EX_DATAERR );
}

/* EOF */
