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
 *  write a mail with subject "Help" to
 *   unix-connect-request@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */


/*
 *  uulog.c
 *
 *  Logfile-Routinen für den ZCONNECT/RFC GateWay
 */

# include "config.h"
# include <stdio.h>
# include <stdarg.h>
# include <time.h>
# include "uulog.h"
# include "ministat.h"

extern char *logdir;


void logfile(char *lfilename, char *from, char *to, char *mid, char *format, ...)
{
	FILE *f;
	time_t now;
	char buf[200];
	va_list ap;

	minireadstat();
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
}
