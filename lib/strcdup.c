/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 2000       Matthias Andree
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
 */


/*
 * strndup Bibliotheksroutine
 */

#include "config.h"

#include "istring.h"

#include "utility.h"

/*@@
 *
 * NAME
 *   strcdup -- String duplizieren, laengenbeschraenkt
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   char *strcdup(const char* from, size_t len)
 *
 * DESCRIPTION
 *   Die Funktion kopiert bis zu len Bytes aus dem NUL-terminierten String
 *   from in einen neu mit malloc angeforderten Speicherbereich.
 *   Wichtig: Das Nullbyte zaehlt mit. Hinweis: es zaehlt die kuerzere Laenge.
 *
 * PARAMETER
 *   const char *from - zu kopierender String.
 *   int len - max. in to zu schreibende Laenge
 *
 * RESULT
 *   Pointer auf die Kopie des Strings, muss mit free() freigegeben werden.
 *   NULL bei Fehler (kein Speicher).
 *
 * BUGS
 *
 */

char *strcdup(const char *from, size_t len)
{
	char *mem, *to;
	size_t mylen = strlen(from) + 1;
	if(len < mylen) mylen = len;
	if(!mylen) return NULL;
	mem = (char *)malloc(mylen);
	if((to = mem)) {
		while(mylen--) {
			if(!(*(to++) = *(from++))) return mem;
		}
		*(to-1)='\0';
	}
	return mem;
}

