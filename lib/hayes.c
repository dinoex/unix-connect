/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999       Matthias Andree
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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 * Waehlroutine fuer ein Hayes-Modem.
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 */

#ifndef ENABLE_MODEM_DEBUG
#define ENABLE_MODEM_DEBUG	1
#endif

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>

#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "ministat.h"
#include "hayes.h"
#include "track.h"
#include "line.h"

/*@@
 *
 * NAME
 *   redial - Anwahl mit einem Hayes-Modem
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include "track.h"
 *   #include "hayes.h"
 *   int redial(const char *anwahl, int modem, int maxtry);
 *
 * DESCRIPTION
 *   Die Funktion fuehrt den in der config-Datei angegebenen Dialog
 *   mit dem Modem und wartet dann,
 *   bis entweder ein nicht behandelbarer Fehler auftritt oder
 *   eine Verbindung zu Stande kommt.
 *
 *   Die Funktion setzt einen alarm-Timer auf 120 Sekunden pro Wahlversuch.
 *   Es muss ein Handler fuer SIGALRM installiert sein.
 *
 * PARAMETER
 *   const char *anwahl - Ein Modemkommando, das einen Waehlvorgang
 *                        ausloest, etwa "ATDT673121".
 *   int         modem  - Eine geoeffnete, korrekt konfigurierte
 *                        tty zum Modem.
 *   int         maxtry - Die maximal durchgefuehrten Anwaehlversuche
 *
 * RESULT
 *   Die Funktion liefert 0, wenn eine Verbindung zustande kommt. Wenn
 *   es zu einem Fehler kommt, ist das Ergebnis 1.
 *
 * BUGS
 *   Der genaue Abbruchgrund ist nicht zu ermitteln.
 *
 */

int
redial (const char *anwahl, int modem, int maxtry)
{
	/* interne Tracking-Nummern      */
	int ok,
	    connect,
	    busy,
	    no_carrier,
	    no_dialtone,
	    no_dial_tone,
	    no_answer,
	    error;
	/* Ergebnis eines Modemkommandos */
	int found;
	/* Pointer in die Konfigurations-Daten */
	header_p conf;

	/* Tracking anmelden */
	ok           = init_track("OK");
	connect      = init_track("CONNECT");
	busy         = init_track("BUSY");
	no_carrier   = init_track("NO CARRIER");
	no_dialtone  = init_track("NO DIALTONE");
	no_dial_tone = init_track("NO DIAL TONE");
	no_answer    = init_track("NO ANSWER");
	error        = init_track("ERROR");

	alarm(45);

	/* Neuer Anfang einer Zeile senden,
	   damit das erste "AT" gleich verstanden wird. */
	write(modem, "\r\n", 2);
	sleep( 1 );
	flush_modem( modem );

	/* Modem Reset */
	conf = find(HD_MODEM_DIALOG, config);
	while (conf) {
		found = do_hayes(conf->text, modem);
		if (found != ok)
			found = do_hayes(conf->text, modem);
		conf = conf->other;
	}
	alarm(0);

	/* Anwahl solange wiederholen, bis sie erfolgreich war */
	do {
		alarm(120);
		maxtry--;
		found = do_hayes(anwahl, modem);
		alarm(0);
		if (found == ok) {
			return 1;
		} else if (found == busy) {
			fputs(" BUSY\n", stderr);
			sleep(1);
		} else if (found == no_carrier)	{
			fputs(" NO CARRIER\n", stderr);
			sleep(1);
		} else if (found == no_answer) {
			fputs(" NO ANSWER\n", stderr);
			sleep(1);
		} else if ((found == no_dialtone) || (found == no_dial_tone)) {
			fputs(" NO DIALTONE\n", stderr);
			return 1;
		}
	} while (found != connect && maxtry > 0);

	/* Tracker wieder freigeben */
	free_all_tracks();

	/* Haben wir's? */
	if (found == connect)
		return 0;
	return 1;
}

/*@@
 *
 * NAME
 *   do_hayes - Sendet ein einzelnes Modemkommando an das Modem.
 *
 * VISIBILITY
 *   Intern zu hayes.c
 *
 * SYNOPSIS
 *   #include "track.h"
 *   #include "hayes.h"
 *   int do_hayes(const char *command, int modem);
 *
 * DESCRIPTION
 *   Die Funktion sendet das gegebene Kommando an das Modem und
 *   liest die Antwort des Modems ein. Dabei wird das Tracking
 *   mitgefuehrt.
 *
 * PARAMETER
 *   const char *command - Das Modemkommando.
 *   int         modem   - Geoeffnete und initialisierte tty des Modems.
 *
 * RESULT
 *   Das Ergebnis der Tracking-Routine.
 *
 * BUGS
 *
 */

int
do_hayes(const char *command, int modem)
{
	char c;    /* Antwort des Modems */
	int found; /* Returncode         */

#ifdef SLOW_MODEM
	/* Dem Modem etwas Ruhe goennen */
       	sleep(1);
#endif

	/* Kommando senden */
	if ( strlen(command) > 0 ) {
		write(modem, command, strlen(command));
		write(modem, "\r", 1);
	}

#if ENABLE_MODEM_DEBUG
	/* Debugging */
	fprintf(stderr, "An Modem : \"%s\"\n", command);
	fprintf(stderr, "Modem    : \"");
#endif

	/* Antwort einlesen, bis der Tracker etwas findet */
	do {
		if ( read(modem, &c, 1) != 1 ) {
			found = -1;
			break;
		}
#if ENABLE_MODEM_DEBUG
		if(isprint(c)) fprintf(stderr, "%c", c);
		else {
			if(c < 0x20) {
				fprintf(stderr, "^%c", c | 0x40);
			} else {
				fprintf(stderr, "\\x%02x", c);
			}
		}
		fflush(stderr);
#endif
		found = track_char(c);
	} while (found < 0);

#if ENABLE_MODEM_DEBUG
	fprintf(stderr, "\"\n");
#endif

	/* Timeout loeschen */
	alarm(0);

	/* Tracker-Ergebnis nach oben melden */
	return found;
}

/* EOF */
