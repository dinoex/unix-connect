/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
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
 *  prologue.c
 *
 *    Der Vorspann...
 */

#include "config.h"
#include "zconnect.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef TCP_SUPPORT
void banner_login(void)
{
	int t, found;

	fputs(TCP_BANNER, stdout);
	t = init_track("zconnect\r");
	fflush(stdout);
	alarm(60);
	for (found = -1; found != t; )
		found = track_char(getc(stdin));
	alarm(0);
}
#endif

void prologue(void)
{
#ifdef ASK_PASSWD
	int found, zc, z, cr_count;
#endif

	alarm(60);
	set_rawmode(fileno(stdin));
	set_rawmode(fileno(stdout));

#ifdef ASK_PASSWD
	zc = init_track("0zconnec\r");

	fputs("\nPasswort: ", stdout);	/* Passwort lesen, dieser Teil	*/
	fflush(stdout);			/* entfaellt je nach Betr.Sys.	*/

	found = -1; cr_count = 0;
	while (found != zc) {
		z = getc(stdin);
		found = track_char(z);
		if (found == zc) break;
		if (z == '\r') {
			cr_count++;
			if (cr_count > 3) {
				fputs("\r\nPasswort: ", stdout);
				fflush(stdout);
				cr_count = 0;
			}
		}
	}
#endif

	alarm(0);
	newlog(INCOMING, "Login %s ZCONNECT erfolgreich", ttyname(0) );

	auflegen = 0;	/* Noch kein Grund zum Verbindungsabbau gefunden  */

	fputs("\r\n" PROGRAM ", v" VERSION
		", (C)opyright 1993 Martin Husemann\r\n"
		"ZCONNECT 3.1 (C)opyright 1992/93 ZERBERUS GmbH, Friedland\r\n"
		, stdout);
	fputs("BEGIN\r\n", stdout); /* Wartezeiten helfen gegen Ueber- */
	fflush(stdout); sleep(1);
	fputs("BEGIN\r\n", stdout); /* tragungsfehler bei < 9600 bps  */
	fflush(stdout); sleep(1);
	fputs("BEGIN\r\n", stdout);
	fflush(stdout);
}
