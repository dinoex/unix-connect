/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1995     Moritz Both
 *  Copyright (C) 1995-98  Christopher Creutzig
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

/* apc_a2b.c - Funktion fuer APC Binaernachrichten-Wandlung */

#include "apc_a2b.h"
#include <string.h>

static const char atob_kennung[] = "\007BINARY FILE FOLLOWS";

/*	long apc_a2b(char *message, long *msglen)

	Stellt fest, ob in dem Text *message eine Binaer-
	Nachricht im APC/ATOB-Format codiert ist. Wenn ja,
	wird sie nach binaer gewandelt. Dabei wird dann
	*msglen auf den neuen, korrekten Wert gesetzt.

	Rueckgabewert: -1, wenn nichts passiert ist
		bei Wandlung: Laenge des Texts, der noch
		vor dem Beginn der Binaerdaten vorhanden
		ist
*/ 

long apc_a2b(char *message, long *msglen) {
	char *cp;

	cp=strchr(message, 7);
	if (cp) {
		long l;
		if (memcmp(cp, atob_kennung, sizeof(atob_kennung)-1))
			return -1L;
		/* Tatsache, das ist eine solche Nachricht */
		l = atob(cp);
		if (l==0) {
			/* ging schief */
			return -1L;
		}
		/* gut gegangen - konvertiert */
		*msglen = l + (cp-message);
		return cp - message;
	}
	return -1L;
}
