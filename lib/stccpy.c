/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1999-2000  Matthias Andree
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
 * stccpy Bibliotheksroutine
 *
 */

#include "config.h"

#include <time.h>

#include "utility.h"

#ifndef HAVE_STCCPY

/*@@
 *
 * NAME
 *   stccpy -- String auf einen anderen kopieren, laengenbeschraenkt
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   size_t stccpy(char *to, const char* from, size_t len)
 *
 * DESCRIPTION
 *   Die Funktion kopiert bis zu len Bytes aus dem NUL-terminierten String
 *   from in den Zielstring to. Der Zielstring ist immer NUL-terminiert.
 *   Das NUL-Byte wird mitgezaehlt.
 *
 * PARAMETER
 *   char *to - String, auf den kopiert werden soll.
 *   const char *from - zu kopierender String.
 *   size_t len - max. in to zu schreibende Laenge
 *
 * RESULT
 *  size_t - Anzahl der geschriebenen Bytes einschl. NUL-Byte
 *
 * BUGS
 *
 */

size_t stccpy(char *to, const char *from, size_t len)
{
	size_t i=0;
	if(!len) return 0;

	while(len) {
		i++;
		if(!(*(to++) = *(from++))) return i;
		len--;
	}
	*(to-1)='\0';
	return i;
}

#endif

/*@@
 *
 * NAME
 *   qstccpy -- String auf einen anderen kopieren, laengenbeschraenkt
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <utility.h>
 *   void qstccpy(char *to, const char* from, size_t len)
 *
 * DESCRIPTION
 *   Die Funktion kopiert bis zu len Bytes aus dem NUL-terminierten String
 *   from in den Zielstring to. Der Zielstring ist immer NUL-terminiert.
 *   Das NUL-Byte wird mitgezaehlt.
 *
 * PARAMETER
 *   char *to - String, auf den kopiert werden soll.
 *   const char *from - zu kopierender String.
 *   size_t len - max. in to zu schreibende Laenge
 *
 * RESULT
 *  void
 *
 * BUGS
 *
 */

void qstccpy(char *to, const char *from, size_t len)
{
	if(!len) return;
	while(len--) {
		if(!(*(to++) = *(from++))) return;
	}
	*(to-1)='\0';
}

/* EOF */
