/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
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

#ifndef SYSDEP_H
#include "sysdep.h"
#endif

#define BIGBUFFER	(256*1024)	/* Kopier-Puffer ( > 30k) */
#define SMALLBUFFER	(64*1024)	/* unused? */
#define	MAXLINE		8192		/* Z.B. in einem SMTP Batch */

/*
 *   receiver und zcall erhalten die Faehigkeit, per TCP/IP miteinander
 *   zu kommunizieren. Dazu muessen BSD sockets vorhanden sein.
 */
#define TCP_SUPPORT

/*
 *   Definiert, falls ZCONNECT-Systeme Daten nicht wieder an RFC-
 *   Systeme zurueckgeben.
 *   Nicht benoetigte UUCP-Header werden sonst komplett transportiert.
 *
 *	DISABLE_UUCP_SERVER
 */

/*
 *   Die folgenden Flags legen fest, bei welchen Konvertierungen eine
 *   Umlautwandlung PC437 --> ISO-8859-1 nicht stattfindet.
 *   Wird das entsprechende Flag definiert, wird ae, oe, ue benutzt...
 *
 *	DISABLE_ISO_IN_MAIL
 *	DISABLE_ISO_IN_NEWS
 */

/*
 *   Das folgende Flag definiert, ob beim Uebergang von RFC -> ZCONNECT
 *   eine ISO-8859-1 --> PC437 Konvertierung stattfindet, wenn die RFC-
 *   Nachricht ISO-8859-1-Zeichen enthaelt.
 *
 *  Das Ganze fuehrt dazu, dass Nachrichten nicht mehr unbedingt unveraendert
 *  durch ZConnect hindurchgereicht werden und sollte mit Vorsicht genossen
 *  werden. Andererseits koennen viele Leute keine ISO-8859-1-Nachrichten
 *  lesen, weil ihre ZConnect-Programme den entsprechenden ZConnect-Header
 *  nicht auswerten.
 */
#define	CONVERT_ISO		/* Bei allen Nachrichten von RFC -> ZCONNECT */


/*
 *   Das folgende ist eine politische Entscheidung und nicht ganz einfach
 *   zu treffen. Ist NO_Z38_CROSS_ROUTING gesetzt, werden News geloescht,
 *   wenn sie
 *
 *      a) Ueber ein Z3.8 System transportiert wurden
 *      b) aber nicht auf einem Z3.8 System geschrieben wurden
 *
 *   Der Normalfall ( RFC -> Z3.8 -> ZCONNECT -> RFC ) ist einsichtig,
 *   leider(?) werden dadurch aber auch Mini-Netze, die nur ueber ein
 *   Z3.8-Gateway angeschlossen sind ausgeschaltet, wenn sie ein Nachrichten-
 *   format benutzen, das nicht transparent nach Z3.8 konvertiert werden
 *   kann.
 *
 *   Diese Option MUSS eingeschaltet werden, wenn auf dem System Bretter
 *   global vernetzt sind (/Z-NETZ/TELEKOM/GATEWAY u.ae.).
 *
 *   Sie darf nicht eingeschaltet werden, wenn dieses System
 *
 *    a) keine Verbindungen ausser der ZCONNECT-Verbindung hat oder
 *    b) es sich hier um einen Point handelt.
 */

#define	NO_Z38_CROSS_ROUTING

/* Der ZConnect-Standard schreibt fuer Message-IDs von Points eine bestimmte
 * Form vor ( <lokalteil>@<point>.<server>.<do>.<main> ). Diese Form
 * kann vom Gateway erzwungen werden. Das kann aber dazu fuehren,
 * dass die Bezugsverkettung im Point nicht mehr sauber funktioniert...
 */
/* #define CARE_FOR_POINT_MIDS */

/* Es gibt in der Behandlung des Routstrings Unterschiede zwischen
 * ZConnect und RFC. Die allgemeine Einigung sieht so aus, dass die
 * RFC-Notation einfach in den ZC-ROT:-Header kopiert wird. Das vermeidet
 * Probleme nach dem Zurueckgaten, auch wenn die Routstrings streng nach
 * ZC falsch sind (der letzte Eintrag muesste verworfen werden).
 * Leider fuehrt das dazu, dass Nachrichten an den Pseudouser MAPS eines
 * Zerberus-Systems nicht anerkannt werden. Daher gibt es die Option,
 * bei Nachrichten an MAPS@* den Routstring auf ZConnect-Format zu stutzen.
 * Das sollte eigentlich keine Probleme machen.
 */
#define CLIP_MAPS_ROT

