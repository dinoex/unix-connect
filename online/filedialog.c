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
 *  write a mail with subject "Help" to
 *   unix-connect-request@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
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
 *  filedialog.c
 *
 *    Steuert den Datenaustausch zwischen den Systemen.
 *    Dies ist der bei weitem komplexeste Teil des ZCONNECT Verfahrens.
 *
 *    Wir wurden angerufen, die andere Seite steuert alles.
 *
 * -------------------------------------------------------------------------
 * Letzte Aenderung: martin@isdn.owl.de, Tue Aug 17 15:18:47 1993
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
#include <sys/types.h>
#include "lib.h"
#include "utility.h"

static unsigned senden = NONE;
static unsigned empfangen = NONE;
static int first = 1; /* noch kein BLK1 in der Dateiphase */

void datei_dialog(void)
{
	header_p req, p, blk2, blk3, blk4;
	int war_get, war_put;
	char txt[10];

	/*
	 *  Diese Stelle ist recht subtil.
	 *  Es ist wichtig, das die MSG_RECV Aktionen, wenn sie vom
	 *  anrufenden System gefordert werden (das ist hier immer die
	 *  Gegenstelle, vergleiche filemaster.c fuer den anderen Fall),
	 *  zuerst ausgefuehrt werden. Durch unsere Art der queue-Verwaltung
	 *  wird die zuletzt gequeuete Aktion bevorzugt - und damit klappt
	 *  es.
	 */
	war_get = war_put = 0;
	senden = NONE;			/* Wir kriegen nichts... */
	empfangen = queue_maske(todo);	/* Was haben wir zu senden? */
	if (empfangen || first) {
		entmaske(txt, empfangen);
		blk2 = new_header(txt, HD_PUT);
	} else
		blk2 = NULL;

	first = 0;

	alarm(30);
	req = get_blk(1);
	alarm(0);
	fputs(__FILE__": BLK1 empfangen:\n", deblogfile);
	wr_para(req, deblogfile);

	/*
	 *  BLK1 wurde korrekt empfangen, wenn vorher Dateien erfolgreich
	 *  uebertragen wurden, duerfen wir sie jetzt loeschen. ZCONNECT
	 *  schreibt diesen Punkt vor, um von exit-Werten der verschiedenen
	 *  Z-MODEM Implementationen unabhaengig zu sein.
	 */
	if (done) {
		remove_queue(done);
		done = clear_queue(done);
	}
	
	p = find(HD_PUT, req);
	if (p) {
		if (*(p->text))
			war_put = 1;
		senden |= maske(p->text);
		fprintf(deblogfile, "Gegenseite moechte Daten liefern: %s (%u)\n", p->text, senden);
		/*
		 *  Dies wird spaeter zuerst ausgefuehrt, bevor unsere Daten
		 *  verschickt werden, da diese schon in der Queue warten und
		 *  wir LIFO abarbeiten.
		 */
		queue_action(connection.tmp_dir, NULL, MSG_RECV, senden, 0);
	}
	p = find(HD_GET, req);
	if (p) {
		war_get = 1;
		empfangen &= maske(p->text);
		fprintf(deblogfile, "Gegenseite moechte Daten abholen: %s (%u)\n", p->text, empfangen);
	}
	p = find(HD_DELETE, req);
	if (p) {
		fprintf(deblogfile, "Gegenseite moechte Daten loeschen: %s\n", p->text);
		queue_action(NULL, NULL, MSG_DELETE, maske(p->text), 1);
	}
	p = find(HD_FILEREQ, req);
	if (p) {
		char dir[FILENAME_MAX], file[FILENAME_MAX], *s;
		time_t eda;

		fprintf(deblogfile, "Gegenseite requested Files: %s\n", p->text);
		sprintf(dir, "%s/%s", fileserverdir, p->text);
		/*
		 *  dir hat jetzt das Format:
		 *     /pub/.../.../../xxx.zip (eda)
		 */

		/*
		 *  Datum ermitteln, falls mitgeschickt. Nur wenn unsere
		 *  Version neuer ist, wird die Datei geschickt.
		 */
/* Momentan nicht funktionsfähig, str2eda ist geändert. */
		s = strrchr(dir, ' ');
		if (s) {
/*			int tz_hour, tz_min;

			eda = str2eda(s+1, &tz_hour, &tz_min); */
			*s = '\0';
		} /* else */
			eda = 0;
		
		s = strrchr(dir, '/');
		if (s) {
			strcpy(file, s+1);
			*s = '\0';
			queue_action(dir, file, FILE_SEND, NONE, 0);
		} else
			fprintf(deblogfile, "Fehlerhafter File-Request: %s\n", dir);
	}
	p = find(HD_FILESEND, req);
	if (p) {
		fprintf(deblogfile, "Gegenseite moechte File senden: %s\n", p->text);
		queue_action(fileserveruploads, p->text, FILE_RECV, NONE, 0);
	}

	if (senden || empfangen || war_put || war_get)
		/*
		 *  Wir haben niemals Bereitstellungszeit.
		 *  Das erspart uns auch das komplizierte "Warten auf Aktion"
		 */
		blk2 = add_header("0", HD_WAIT, blk2);
	else {
		blk2 = add_header("Y", HD_LOGOFF, blk2);
		auflegen = 1;
	}
	blk2 = put_blk(blk2, 2);
	fputs("BLK2 gesendet:\n", deblogfile);
	wr_para(blk2, deblogfile);
	if (auflegen) {
		fflush(stdout);
		return;
	}

	alarm(30);
	blk3 = get_blk(3);
	alarm(0);
	fputs(__FILE__": BLK3 empfangen:\n", deblogfile);
	wr_para(blk3, deblogfile);

	blk4 = new_header((senden||empfangen) ? "Y" : "N", HD_EXECUTE);
	fputs("Sende BLK4:\n", deblogfile);
	wr_para(blk4, deblogfile);
	blk4 = put_blk(blk4, 4);
}

