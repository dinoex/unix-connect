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



/********************************************************************
 *
 *   File    :  delheader.c
 *   Autoren :  Martin
 *   Funktion:  L”scht einen Header
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "header.h"

header_p del_header(unsigned int code, header_p ptr)
{
        header_p p, last;


        /* Loeschstelle suchen */
        last = NULL;
	for (p = ptr; p && (p->code < code); p = p->next)
		last = p;

	if (!p || (p->code != code)) {
		/* nicht gefunden -> gar nichts loeschen */
		return ptr;
	}

	if (!last) {
		/* Ganz am Anfang loeschen */
		p = ptr;
		ptr = ptr->next;
	} else {
		/* Irgendwo mittendrin loeschen */
                while (last)             /* Alle Other-Pointer richtig loeschen. */
                {                        /* Der Autor wird zu 20 Ave Maria */
                                         /* verurteilt. Gnarf. */
                   last->next = p->next;
                   last = last->other;
                }

        }

        /* Header p und alle gleichnamigen loeschen */
        while (p) {
                if (p->text) dfree (p->text);
                last = p->other;
                dfree(p);
                p = last;
        }

        return ptr;
}
