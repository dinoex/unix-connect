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
 *   track.c
 *
 *   track.c implementiert einen Knuth-Morris-Pratt Algorithmus
 *   zur simultanen Patternsuche in mehreren String ohne Backup.
 *
 */

#include "config.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include "track.h"

/* KMP-Array */
static short next[MAXTRACKS][MAXPATTERN];

/* Suchstrings */
static char *pat[MAXTRACKS] = { NULL };

/* Laenge des Suchstrings, Position im String */
static int   m[MAXTRACKS], gj[MAXTRACKS];

/* Letzte belegte Track */
static int   last_track = 0;

/*@@
 *
 * NAME
 *   init_track -- Initialisiert den Such-Automaten und uebergibt das zu
 *                 suchende Pattern.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <track.h>
 *   int init_track(const char *pattern);
 *
 * DESCRIPTION
 *   Die Funktion registiert den Suchstring und vergibt fuer ihn eine
 *   interne Nummer. Fuer den Suchstring wird ein KMP-Backup
 *   eingerichtet. Vergleiche Sedgewick, "Algorithms", pp. 280-285
 *   und pattern.[ch].
 *
 * PARAMETER
 *   const char *pattern - Ein Suchstring.
 *
 * RESULT
 *   Die interne Nummer des Suchpatterns. -1, wenn alle Kanaele belegt
 *   sind.
 *
 * BUGS
 *   Die Funktion arbeitet auf einem statischen Feld. Es koennen maximal
 *   MAXTRACKS (16) Strings von MAXPATTERN (256) Zeichen Laenge auf einem
 *   Suchkanal aktiv sein.
 *
 */

int init_track(const char *pattern)
{
	int i, j, pat_nr;

	/* Suche ein leeres Pattern */
	for (pat_nr = 0; pat_nr < last_track; pat_nr++) {
		if (pat[pat_nr] == NULL)
			break;
	}

	/* Es war kein leeres Pattern zwischendrin frei */
	if (pat_nr >= last_track) {
		if (last_track <= MAXTRACKS) {
			last_track++;
		} else {
			return -1;
		}
	}

	/* Suchstring merken */
	pat[pat_nr] = dstrdup(pattern);
	m[pat_nr] = strlen(pattern);

	/* procedure initnext, Sedgewick p. 282 */
	i = 0; j = -1; next[pat_nr][0] = -1;
	do {
		if ((j<0) || (pattern[i] == pattern[j])) {
			i++;
			j++;
			if (pattern[i] != pattern[j])
				next[pat_nr][i] = j;
			else
				next[pat_nr][i] = next[pat_nr][j];
		} else {
			j = next[pat_nr][j];
		}
	} while (i <= m[pat_nr]);

	for (i = 0; i <= m[pat_nr]; i++)
		if (next[pat_nr][i] < 0)
			next[pat_nr][i] = 0;

	/* Position im String auf Stringanfang setzen */
	gj[pat_nr] = 0;

	return pat_nr;
}

/*@@
 *
 * NAME
 *   track_char -- Ein Zeichen fuer die KMP-Suche uebergeben.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   int track_char(const char input);
 *
 * DESCRIPTION
 *   Es wird geprueft, ob das uebergebene Zeichen auf den belegten
 *   Tracks ein Muster vervollstaendigt.
 *
 * PARAMETER
 *   const char input - das Zeichen.
 *
 * RESULT
 *   Die Nummer der Track, auf der des Zeichen ein Pattern vervollstaendigt.
 *   -1, wenn kein Pattern gefuellt wurde.
 *
 * BUGS
 *   Die Funktion arbeitet auf statischen Feldern. Es koennen maximal
 *   MAXTRACKS (16) Tracks mit je MAXPATTERN (256) Zeichen langen Strings
 *   auf einem Suchkanal aktiv sein.
 *
 */

int track_char(const char input)
{
	int i, found;

	/* Resultat vorbelegen */
	found = -1;

	/* KMP Suche, Sedgewick p. 282 */
	for (i=0; i<last_track; i++) {
		if (pat[i] == NULL)		/* track was freed, */
			continue;		/* skip this one */
		if (input == pat[i][gj[i]]) {
			gj[i]++;
		} else {
			gj[i] = next[i][gj[i]];
		}
		/* Wir haben was gefunden */
		if (gj[i] >= m[i]) {
			found = i;
			gj[i] = 0;
		}
	}
	return found;
}

/*@@
 *
 * NAME
 *   free_track -- Freigeben einer Track.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   void free_track
 *
 * DESCRIPTION
 *   Entfernt das Pattern auf der internen Suchstruktur und gibt diesen
 *   Eintrag wieder frei.
 *
 * PARAMETER
 *   const int patnr - Nummer der Track, die freizugeben ist.
 *
 * RESULT
 *   keins.
 *
 * BUGS
 *
 */

void free_track(const int patnr)
{
	if (pat[patnr] != NULL) {
		dfree(pat[patnr]);
		pat[patnr] = NULL;
		m[patnr] = 0;
		gj[patnr] = 0;
		next[patnr][0] = 0;
	}
}

/*@@
 *
 * NAME
 *   free_all_tracks -- Alle Tracks freigeben.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   void free_all_tracks(void);
 *
 * DESCRIPTION
 *   Die Funktion gibt alle noch belegten Tracks frei.
 *
 * PARAMETER
 *   keine.
 *
 * RESULT
 *   keins.
 * BUGS
 *
 */

void free_all_tracks(void)
{
	int i;

	for(i=0; i<MAXTRACKS; i++)
		free_track(i);
	last_track = 0;

	return;
}

/* EOF */
