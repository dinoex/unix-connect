/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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
 *  christopher@nescio.foebud.org or snail-mail:
 *  Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  to join, ask Nora Etukudo at
 *   nora.e@mailinglisten.im-netz.de
 *
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
 */

# include "config.h"
# include <stdio.h>
# include <stdarg.h>
# include <time.h>
# include "ministat.h"
# include "uulog.h"

char *nomem = "Nicht genug Hauptspeicher!";

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


void uufatal(char *prog, char *format, ...)
{
	FILE *f;
	time_t now;
	char buf[200];
	va_list ap;

	minireadstat();
	sprintf(buf, "%s/" ERRLOG, logdir);
	f = fopen(buf, "a");
	if (!f) {
		fprintf(stderr, "\nFATAL: Logfile \""ERRLOG"\" nicht schreibbar\n");
		exit(10);
	}
	now = time(NULL);
	strftime(buf, 200, "%Y/%m/%d %H:%M:%S", localtime(&now));
	fprintf(f, "%s:\t", prog);
	va_start(ap, format);
	vfprintf(f, format, ap);
	va_end(ap);
	fputs("\n", f);
	fclose(f);
	fprintf(stderr, "\n\n%s:\t", prog);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputs("\n", stderr);

	exit(10);
}



/* EOF */
