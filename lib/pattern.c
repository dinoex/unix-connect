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
 *   pattern.c
 *
 *   Pattern.c implementiert einen Knuth-Morris-Pratt Algorithmus
 *   zur Patternsuche in einem String ohne Backup.
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif

#include "pattern.h"
#include "lib.h"

static short next[MAXPATTERN];
static char *pat = NULL;  /* Suchstring */
static int m,             /* Laenge des Suchstrings pat */
	   gj;            /* Bisher gefundene Zeichen   */

/*@@
 *
 * NAME
 *   init_search - Initialisiert den Such-Automaten und uebergibt das zu
 *                 suchende Pattern.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <pattern.h>
 *   int init_search(const char *pattern);
 *
 * DESCRIPTION
 *   KMP ist beschrieben in "Algorithms", Sedgewick, pp. 280-285.
 *
 * PARAMETER
 *   const char *pattern - Zu suchender String.
 *
 * RESULT
 *   Immer Null.
 *
 * BUGS
 *   Die Puffer sind statisch, d.h. es kann nur eine KMP-Suche zur
 *   Zeit aktiv sein.
 *
 */

int init_search(const char *pattern)
{
	int i, j;

	/* Suchstring kopieren */
	if (pat != NULL)
		dfree(pat);
	pat = dstrdup(pattern);

	m = strlen(pattern);

	/* Siehe "procedure initnext", p. 283 im Sedgewick */
	i = 0; j = -1; next[0] = -1;
	do {
		if ((j<0) || (pattern[i] == pattern[j])) {
			i++;
			j++;
			if (pat[i] != pat[j])
				next[i] = j;
			else
				next[i] = next[j];
		} else {
			j = next[j];
		}
	} while (i <= m);

	for (i = 0; i <= m; i++)
		if (next[i] < 0)
			next[i] = 0;

	/* Positionszeiger auf Stringstart setzen */
	gj = 0;

	return 0;
}

/*@@
 *
 * NAME
 *   search_char - Uebergebe das naechste Zeichen fuer unsere KMP-Suche.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   int search_char(const char input);
 *
 * DESCRIPTION
 *   Die Funktion vergleicht das uebergebene Zeichen mit dem KMP-
 *   Puffer unter Beruecksichtigung vorheriger Aufrufe und signalisiert,
 *   ob schon etwas gefunden wurde.
 *
 *   Die Funktion arbeitet auf einem statischen Puffer. Es kann nur
 *   eine KMP-Suche zur Zeit aktiv sein.
 *
 * PARAMETER
 *   const char input - das naechste Zeichen fuer KMP-Suche.
 *
 * RESULT
 *   Die Funktion liefert 0, solange das Pattern noch nicht gefunden wurde,
 *   eine 1, wenn das letzte Zeichen das Pattern vervollstaendigte.
 *
 * BUGS
 *   Die Funktion arbeitet mit statischen Puffern.
 *
 */

int search_char(const char input)
{
	/* procedure kmpsearch, Sedgewick p. 282 */
	if (input == pat[gj]) {
		gj++;
	} else {
		gj = next[gj];
	}

	/* Haben wir was gefunden ? */
	if (gj >= m)
		return 1;
	else
		return 0;
}
