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


/********************************************************************
 *
 *   File    :  freepara.c
 *   Autoren :  Martin
 *   Funktion:  Gibt einen Header wieder frei
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "header.h"


void do_free_para(header_p ptr)
{
        header_p ptr1, next;


        next = NULL;
        while (ptr) {
                if (!next)
                        next = ptr->next;
#ifdef DEBUG
                else if (next != ptr->next)
                        uufatal (__FILE__, "Headerverwaltung hat noch Bugs.");
#endif
                if (ptr->other)
                        ptr1 = ptr->other;
                else
                {
                        ptr1 = next;
                        next = NULL;
                }

                if (ptr->text) dfree(ptr->text);
                dfree(ptr);
                ptr = ptr1;
        }
}

