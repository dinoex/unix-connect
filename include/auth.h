/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1999-2000  Andreas Barth
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
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

typedef struct {
	const char *ssysname, *lsysname;
                /* Name des eingeloggten Systems (kurz und lang) */
	const char *arcerout, *arcerin, *transfer;
                /* Die beiden Packer und das Uebertragungsprotokoll */
} auth_t;

auth_t*
authsys(const int version, const char *sysname, const char *passwd, const char *sysdir);

#define E_ALARM 1
#define E_HUP 2
#define E_WRONGVERSION 3
#define E_NOSYS 4 /* System existiert nicht */
#define E_BROKENSYS 5 /* dazugehoerige Datei kaputt */
#define E_NOACCESS 6 /* falsches Passwort */
#define E_NOARCERO 7 /* kein Arcer rausgehend */
#define E_NOARCERI 8 /* kein Arcer eingehend */
#define E_NOPROTO 9 /* kein Uebertragungsprotokoll */
#define E_NODIR 10 /* kann tmp-Verzeichnis nicht anlegen */

