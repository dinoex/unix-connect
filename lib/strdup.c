/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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
 * strdup Bibliotheksroutine
 *
 * NeXTstep 3.0 kennt kein strdup().
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  KK  Dokumentation erstellt.
 *
 */

#include "config.h"
#include "utility.h"

#ifdef HAS_STRING_H
#include <string.h>
#endif
#ifdef HAS_STRINGS_H
#include <strings.h>
#endif

#include "uulog.h"

#if defined(NEXTSTEP)

/*@@
 *
 * NAME
 *   strdup -- Stringskopie anfertigen.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   char *strdup(const char *a);
 *
 * DESCRIPTION
 *   Die Funktion reserviert Speicher mit malloc() und kopiert den
 *   uebergeben String in diesen Speicherbereich.
 *
 * PARAMETER
 *   const char *a - zu kopierender String.
 *
 * RESULT
 *  char * - Kopie des Strings
 *
 * BUGS
 *
 */

char *strdup(const char *a)
{
    int len = strlen(a);
    char *p = (char *) malloc(len);
    if ( p == NULL)
	return p;

    strcpy(p, a);
    return p;
}

#endif

char *dstrdup(const char *p)
{
	char *erg;

	erg = strdup(p);
	if (!erg) {
		out_of_memory(__FILE__);
	}

	return erg;
}

/* EOF */
