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
 *   File    :  joinheader.c
 *   Autoren :  Martin
 *   Funktion:  Verbindet zwei Header zu einem
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif

#include "utility.h"
#include "crc.h"
#include "header.h"
#include "uulog.h"

/*
 *  Erzeugt die Kopie eines other-> Astes, behaelt dabei die
 *  Reihenfolge bei.
 */
header_p copy_one_header(header_p p)
{
        header_p p2, start, last;

	start = NULL; last = NULL;
        while (p) {
                p2 = (header_p)dalloc(sizeof(header_t));
                if (!start) start = p2;
                if (last) last->other = p2;
                last = p2;
                *p2 = *p;
                p2->next = NULL;
                p2->other = NULL;
                p2->text = strdup(p->text);
                if (!p2->text) out_of_memory(__FILE__);
                p = p->other;
        }

        return start;
}

/*
 * join_header wirkt wie eine Addition. Es wird ein Header erzeugt, der aus
 * den Daten der beiden anderen Headern besteht. Die beiden uebergebenen
 * Header werden dabei nicht geaendert.
 *
 * Die Reihenfolge der other-Verkettung bleibt dabei erhalten.
 */

header_p join_header(header_p p1, header_p p2)
{
        header_p start, p, new, tmp;

        if (!p1 && !p2)
                uufatal(__FILE__, "Nachrichtenkopf: interner Fehler #42");

        /* Sentinel am Anfang der Liste erzeugen */
        start = (header_p)dalloc(sizeof(header_t));
	p = start; p->next = NULL; p->other = NULL;

        /* die beiden Listen abarbeiten */
        while (p1 || p2) {
                if (!p1 || (p2 && (p1->code >= p2->code))) {
                        new = copy_one_header(p2);
                        p2 = p2->next;
                        if (p1 && (p1->code == new->code)) {
                                for (tmp = new; tmp->other; tmp = tmp->other)
                                        ;
                                tmp->other = copy_one_header(p1);
                                p1 = p1->next;
                        }
                } else {
                        new = copy_one_header(p1);
                        p1 = p1->next;
                }
                for (tmp = p; tmp; tmp = tmp->other)
                     tmp->next = new;
                p = new;
        }
        p->next = NULL;

        /* Sentinel wieder loeschen */
        p = start->next;
        dfree (start);
        return p;
}
