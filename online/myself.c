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
 *  ZCONNECT fuer UNIX, (C) 1993 Martin Husemann, (C) 1995 Christopher Creutzig
 *
 *  Sie sollten mit diesen Quelltexten eine Kopie der GNU General Public
 *  Licence erhalten haben (Datei COPYING).
 *
 *  Diese Software ist urheberrechtlich geschuetzt. Sie ist nicht und war
 *  auch niemals in der Public Domain. Alle Rechte vorbehalten.
 *
 *  Sie duerfen diese Quelltexte entsprechend den Regelungen der GNU GPL
 *  frei nutzen.
 *
 * ------------------------------------------------------------------------
 *
 *  myself.c:
 *
 *    Ermittelt alle Daten ueber das lokale ZCONNECT System
 *
 */

#include "config.h"
#include "zconnect.h"
#include <errno.h>

/*
 *   get_myself() : liest die Daten der eigenen Box ein
 */
header_p get_myself(void)
{
	FILE *f;
	header_p p;

	f = fopen(myself, "r");
	if (!f) {
		uufatal(myself, "%s\n", strerror(errno));
		exit(1);
	}
	p = rd_para(f);
	fclose(f);

	return p;
}

