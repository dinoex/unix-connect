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
 * case-insensitive string compare.
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 */

/*
 * Eigentlich gibt es doch strcasecmp? ccr
 */

#include "config.h"

#include <time.h>
#include <ctype.h>

#include "utility.h"

/*@@
 *
 * NAME
 *   stricmp -- Stringsvergleich ohne Beruecksichtigung von Gross- und
 *              Kleinschreibung.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   int stricmp(char *a, char *b);
 *
 * DESCRIPTION
 *   Die Funktion arbeitet analog zu strcmp, jedoch ohne Beruecksichtigung
 *   der Gross- und Kleinschreibung.
 *
 * PARAMETER
 *   char *a, *b - die zu vergleichenden Strings.
 *
 * RESULT
 *   -1 wenn a<b, 0 wenn a==b, 1 wenn a>b
 *
 * BUGS
 *   strcmp liefert als Ergebnis (*a)-(*b), einige Anwendungen
 *   moegen das benutzen. a und/oder b duerfen nicht NULL sein.
 */

int stricmp(const char *a, const char *b)
{
	/* Gleiche Zeichen ueberspringen */
	while (*a && *b && toupper(*a) == toupper(*b))
		a++, b++;

	/* Vergleichen */
	if (!*a && !*b)
	    return 0;
	else if (toupper(*a) < toupper(*b))
		return -1;
	else return 1;
}

/* EOF */
