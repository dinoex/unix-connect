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


/*
 *  sysmaster.c:
 *
 *    Austausch der Systeminformationen zwischen den beiden Systemen.
 *    Wir sind der Anrufer und haben die freie Wahl...
 */

#include "config.h"
#include "zconnect.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include "lib.h"

int ist_testaccount = 0;

/*
 *   system_master tauscht die Informationen ueber die beteiligten Systeme
 *   aus.
 */
void system_master(header_p hmyself, header_p remote)
{
	header_p p, p1, blk2, blk3, blk4;

	fputs("\nSystemparameter werden ausgetauscht\n", stderr);

	/*
 	 *  Den eigenen Systemnamen gross schreiben (ist nicht vorgeschrieben)
 	 *  sowie Passwort und unseren Arcer fuer dieses System eintragen.
 	 *  Achtung: ARCEROUT ist missverstaendlich: in unserer Systme-Datei
 	 *	     heisst es: der ARCer, den WIR ausgehend verwenden. Es
 	 *	     ist gegenueber den vom fremden System gesendeten Bloecken
 	 *	     genau vertauscht!
	 */
	p = find(HD_SYS, hmyself);
	if (p) strupr(p->text);
	p = find(HD_PASSWD, remote);
	if (p) hmyself = add_header(p->text, HD_PASSWD, hmyself);
	p = find(HD_ARCEROUT, remote);
	if (p) hmyself = add_header(p->text, HD_ARCEROUT, hmyself);
	/*
	 *  Ein paar Spezial-Header, die wir in der systems-Datei speichern
	 *  werden entfernt. Die Gegenseite wuerde sie sowiso ignorieren.
	 */
	hmyself = del_header(HD_X_CALL, hmyself);
	hmyself = del_header(HD_POINT_USER, hmyself);

	hmyself = put_blk(hmyself, 1);

	fprintf(deblogfile, "Sende BLK1: \n");
	wr_para(hmyself, deblogfile);

	alarm(30);
	blk2 = get_blk(2);
	alarm(0);
	fputs("BLK2 empfangen:\n", deblogfile);
	wr_para(blk2, deblogfile);

	/*
	 *  Es ist sinnvoll, hier zu testen, ob das System auf
	 *  der Gegenseiten den erwarteten Namen im SYS: Header geschickt
	 *  hat. Wer ZCONNECT-Autoeintrag akzeptiert hat so zumindest die
	 *  Sicherheit, unter der angegeben Nummer auch das erwarete System
	 *  zu erreichen.
	 */
	p = find(HD_SYS, remote);
	p1 = find(HD_SYS, blk2);
	if (!p || !p1 || stricmp(p->text, p1->text) != 0) {
		logoff("Gegenseite ist nicht das erwartete System");
		put_blk(NULL, 3);
		return;
	}

	if (auflegen_empfangen) {
		fputs("Logoff durch die Gegenseite\n", stderr);
		return;
	}

	blk3 = sysparam(hmyself, blk2, remote);
	/*
	 *  Falls blk3 == NULL ist, wurde von sysparam() bereits der
	 *  Fehler bei "logoff()" eingetragen. Es wird dann hier ein
	 *  leerer Block mit der Fehlermeldung gesendet.
	 */
	fputs("BLK3 gesendet:\n", deblogfile);
	wr_para(blk3, deblogfile);
	blk3 = put_blk(blk3, 3);

	alarm(30);
	blk4 = get_blk(4);
	alarm(0);

	fputs("Blk4 empfangen:\n", deblogfile);
	wr_para(blk4, deblogfile);

	/*
	 *  Hier koennte man nochmal pruefen, ob die Gegenseite nicht
	 *  Bloedsinn gesendet hat (z.B. Packer, die es hier nicht gibt).
	 */
	p = find(HD_PROTO, blk4);
	if (p) {
		connection.proto = dstrdup(p->text);
	}
	p = find(HD_ARCEROUT, blk4);
	if (p) {
		connection.remote_arc = dstrdup(p->text);
	}
	p = find(HD_ARCERIN, blk4);
	if (p) {
		connection.local_arc = dstrdup(p->text);
	}

	free_para(blk2);
	free_para(blk3);
	free_para(blk4);

	fprintf(stderr, "Protokoll: %s   Packer hier: %s     Packer Gegenseite: %s\n",
		connection.proto, connection.local_arc, connection.remote_arc);
}
