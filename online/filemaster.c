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
 * -------------------------------------------------------------------------
 *
 *  filemaster.c
 *
 *    Steuert den Datenaustausch zwischen den Systemen.
 *    Dies ist der bei weitem komplexeste Teil des ZCONNECT Verfahrens.
 *
 *    Wir haben angerufen und muessen auch die Kontrolle uebernehmen...
 *
 * -------------------------------------------------------------------------
 *
 *    Letzte Aenderung: christopher@nescio.zerberus.de, 14:45, 22. Nov. 1995
 */

#include "config.h"
#include "zconnect.h"
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

static unsigned senden = NONE;
static unsigned empfangen = ALL;

static int not_done = 0;		/* Letzter Transfer wurde abgelehnt */
static int is_z51 = 0;			/* ZERBERUS 5.1 (buggy) auf der Gegenseite */
static int more_data = 1;		/* Gegenseite hat noch was */
extern int wait_sec;
action_p senden_queue;

void datei_master(header_p hmyself, header_p remote)
{
	header_p req, p, blk2, blk3, blk4;

	alarm(60);
	req = NULL;
	if (more_data) {
		senden = NONE;
		req = add_header("PEBF", HD_GET, req);
	} else {
		if (senden_queue) {
			if (todo) todo = clear_queue(todo);
			todo = senden_queue;
			senden_queue = NULL;
		}
		senden = queue_maske(todo);	/* Was haben wir? */
		if (senden) {
			char txt[20];

			entmaske(txt, senden);
			req = add_header(txt, HD_PUT, req);
		}
	}

	req = put_blk(req, 1);
	fputs("Sende BLK1:\n", deblogfile);
	wr_para(req, deblogfile);

	blk2 = get_blk(2);

	/*
	 * Ok, hier war ein ordnungsgemaesser Header-Austausch,
	 * also falls wir bei der letzten Runde etwas uebertragen
	 * haben, duerfen wir es jetzt loeschen:
	 */
	if (done) {
		remove_queue(done);
		done = clear_queue(done);
	}

	fputs("BLK2 empfangen:\n", deblogfile);
	wr_para(blk2, deblogfile);
	empfangen = NONE;
	p = find(HD_PUT, blk2);
	if (!p) {
		if (!is_z51) {
			fputs("Gegenseite ist ZERBERUS 5.x [Bugmodus aktiviert]\n", stderr);
			fputs("Gegenseite ist ZERBERUS 5.x [Bugmodus aktiviert]\n", deblogfile);
			is_z51 = 1;
		}
		if (more_data)
			empfangen = ALL;
	} else {
		/*
		 *  Normales System auf der Gegenseit:
		 *
		 *    Es verraet uns, was wir von ihm bekommen werden.
		 */
		empfangen = maske(p->text);
		if (!empfangen) more_data = 0;
	}
	if (more_data) {
		fprintf(deblogfile, "Gegenseite hat Daten, Auftrag in der queue: Empfangen\n");
		queue_action(connection.tmp_dir, NULL, MSG_RECV, empfangen, 0);
	}

	waittime=0;
	p = find(HD_WAIT, blk2);
	wait_sec=0;
	if (p) {
		waittime = (unsigned)atol(p->text);
		if (waittime > 10) {
			fprintf(stderr, "Bereitstellungszeit der Gegenseite: %u Sekunden\n", waittime);
		}
	}

	if (auflegen_empfangen) {
		fputs("Fertig, auflegen\n", deblogfile);
		auflegen = 1;
		alarm(0);
		free_para(req);
		free_para(blk2);
		return;
	}
	not_done = 0;

	if (!more_data && !senden) {
		blk3 = new_header("Y", HD_LOGOFF);
		blk3 = add_header("N", HD_EXECUTE, blk3);
		auflegen_gesendet = 1;
	} else
		blk3 = new_header("Y", HD_EXECUTE);
	fputs("Sende BLK3:\n", deblogfile);
	wr_para(blk3, deblogfile);
	fflush(deblogfile);
	blk3 = put_blk(blk3, 3);

	blk4 = get_blk(4);
	alarm(0);
	fputs("BLK4 empfangen:\n", deblogfile);
	wr_para(blk4, deblogfile);

	if (auflegen_empfangen || auflegen_gesendet) {
		fputs("Fertig, auflegen\n", deblogfile);
		auflegen = 1;
		free_para(req);
		free_para(blk2);
		free_para(blk3);
		free_para(blk4);
		return;
	}

	p = find(HD_EXECUTE, blk4);
	if (p) {
		not_done = stricmp(p->text, "Y") != 0;
		if (not_done && is_z51) {
			more_data = 0;
			todo = clear_queue(todo);
		}
		fprintf(deblogfile, "EXECUTE: %s, not_done = %d, more_data = %d\n", p->text, not_done, more_data);
	}
	free_para(req);
	free_para(blk2);
	free_para(blk3);
	free_para(blk4);
}

