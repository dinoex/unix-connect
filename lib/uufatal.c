/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1994     Peter Much
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
 *  UUfatal.c
 *
 *  Logfile-Routinen fuer den ZCONNECT/RFC GateWay
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <sysexits.h>

#include "uulog.h"

#define	MAX_LOG_LINE	200

const char *nomem = "Nicht genug Hauptspeicher!";

/*@@
 *
 * NAME
 *   uufatal -- Fatalen Fehler loggen.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include <uulog.h>
 *   void uufatal(char *prog, char *format, ...);
 *
 * DESCRIPTION
 *   Die Funktion loggt den aufgetretenen Fehler mit Datum und beendet
 *   das Programm.
 *
 * PARAMETER
 *   char *prog -- Name des verstorbenen Programms.
 *
 * RESULT
 *   Die Funktion kehrt nie zurueck.
 *
 * BUGS
 *
 */


void
uufatal(const char *prog, const char *format, ...)
{
	char buf[ MAX_LOG_LINE ];
	va_list ap;

	va_start(ap, format);
	vsnprintf( buf, MAX_LOG_LINE, format, ap );
	va_end(ap);
	newlog( ERRLOG, "%s: %s", prog, buf );
	exit( EX_DATAERR );
}

/* EOF */
