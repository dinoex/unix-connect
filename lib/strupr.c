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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 * strupr - String nach Uppercase wandeln
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 */

/*
 * toupper() ist doch ANSI-C? ccr
 */


#include "config.h"

#include <time.h>
#include <ctype.h>

#include "utility.h"

/*@@
 *
 * NAME
 *   strupr -- String nach Uppercase wandeln
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   char *strupr(char *a);
 *
 * DESCRIPTION
 *   Die Funktion ueberschreibt den gegebenen String mit seinem
 *   Aequivalent in Uppercase.
 *
 * PARAMETER
 *   char *a - zu wandelnder String.
 *
 * RESULT
 *   char * - Der gewandelte String.
 *
 * BUGS
 *   Die Funktion testet nicht auf a == NULL.
 *
 */

char *strupr(char *a)
{
	char *p, c;

	for (p = a; *p; p++)
		if (isascii(*p)) {
			c = *p;
			if (islower(c))
				c = toupper(c);
			*p = c;
		}
	return a;
}
