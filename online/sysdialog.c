/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999-2000  Dirk Meyer
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


/*
 *  sysdialog.c:
 *
 *    Austausch der Systeminformationen zwischen den beiden Systemen.
 *    Wir wurden angerufen und hoeren auf die andere Seite (soweit das geht).
 */

#include "config.h"
#include "zconnect.h"

#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "utility.h"

int ist_testaccount;

/*
 *   system_dialog tauscht die Informationen ueber die beteiligten Systeme
 *   aus.
 */
void system_dialog(void)
{
	header_p anrufer, ich, p, soll, blk3, blk4, d;
	char sysname[FILENAME_MAX], fname[FILENAME_MAX], *pw_ist, *pw_soll,
		*my_arc, *my_proto, *blk3_arc, *blk3_proto, *domain;
	int ist_autoeintrag, ist_gast;
	FILE *f;

	soll = 0;
	ist_testaccount = 0;
	ist_autoeintrag = 0; /* Dies ist kein Autoeintrag ... */
	alarm(60);
	anrufer = get_blk(1);
	alarm(0);
	if (auflegen_empfangen)
		return;
	if (!anrufer) {
		fputs("Keine Daten des anrufenden Systems ermittelt\n", deblogfile);
		fclose(deblogfile);
		exit(3);
	}
	fputs("\nBLK1 beendet,\nDaten des anrufenden Systems:\n", deblogfile);
	wr_para(anrufer, deblogfile);

	/*
	 *  Soll-Daten der Gegenseite ermitteln:
	 */
	p = find(HD_SYS, anrufer);
	if (!p) {
		logoff("Protokoll-Fehler: kein "HN_DOMAIN": Header (System-Name)");
		put_blk(NULL, 2);
		return;
	}
	d = find(HD_DOMAIN, anrufer);
	if (!d) {
		logoff("Protokoll-Fehler: kein "HN_DOMAIN": Header (System-Domains)");
		put_blk(NULL, 2);
		return;
	}
	for (domain = strtok(d->text, " ;:"); domain; domain = strtok(NULL, " ;,")) {
		sprintf(sysname, "%s.%s", p->text, domain);
		strlwr(sysname);
		sprintf(fname, "%s/%s", systemedir, sysname);
		if (access(fname, R_OK) == 0) break;
	}
	if (access(fname, R_OK) != 0) {
		sprintf(sysname, "%s.%s", p->text, d->text);
		strlwr(sysname);
	}

	connection.remote_sys = dstrdup(sysname);
	sprintf(fname, "%s/zr.%s.%d", netcalldir, sysname, getpid());
	mkdir(fname, 0777);
	connection.tmp_dir = dstrdup(fname);
	fprintf(deblogfile, "Anrufendes System: %s\n", sysname);
	sprintf(fname, "%s/%s", systemedir, sysname);
	ist_gast = 0;
	f = fopen(fname, "r");
	if (!f) {
		fputs("System-Datei nicht gefunden\n", deblogfile);
		p = find(HD_PASSWD, anrufer);
		if (p && (stricmp(p->text, "guest") == 0 ||
			stricmp(p->text, "gast") == 0))
			ist_gast = 1;
		if (!ist_gast) {
			if (autoeintrag) f = fopen(autoeintrag, "r");
			if (!autoeintrag || !f) {
				fputs("Autoeintrag ist nicht konfiguriert!\n", deblogfile);
				logoff("Autoeintrag nicht moeglich");
				put_blk(NULL, 2);
				return;
			}
		}
		ist_autoeintrag = 1;
	} else {
		soll = rd_para(f);
		fclose(f);
	}
	if (ist_autoeintrag) {
	   if (!ist_gast) {
		fputs("Autoeintrag: speichere die (temporaeren) Daten\n", deblogfile);
		f = fopen(fname, "w");
		if (!f) {
			fputs("Autoeintrag nicht moeglich!\n", deblogfile);
			logoff("Autoeintrag ist fehlgeschlagen");
			put_blk(NULL, 2);
			return;
		}
		fputs("X-Call: ZCONNECT\n", f);
		p = find(HD_ARCERIN, anrufer);
		if (!p) p = find(HD_ARCEROUT, anrufer);
		if (p)
			anrufer = add_header(p->text, HD_ARCERIN, anrufer);
		anrufer = del_header(HD_STATUS, anrufer);
		anrufer = del_header(HD_CRC, anrufer);
		wr_para(anrufer, f);
		fclose(f);
		ist_testaccount = 0;
	   }
	} else {
		pw_ist = pw_soll = NULL;
		p = find(HD_PASSWD, anrufer);
		if (p) pw_ist = p->text;
		p = find(HD_PASSWD, soll);
		if (p) pw_soll = p->text;
		if (!pw_ist)
			fprintf(deblogfile, "Anrufer sendet kein Passwort!\n");
		if (!pw_soll)
			fprintf(deblogfile, "Systemeintrag ohne Passwd: %s\n", sysname);
		if (strcmp(pw_ist, pw_soll) == 0)
			fprintf(deblogfile, "Passwort OK\n");
		else {
			fprintf(deblogfile, "Passwort falsch: %s / %s\n",  pw_ist, pw_soll);
			logoff("Passwort falsch");
			put_blk(NULL, 2);
			return;
		}
		ist_testaccount = ( find(HD_X_TESTACCOUNT, soll) != NULL);
	}

	ich = get_myself();

	my_arc = my_proto = NULL;
	p = find(HD_PROTO, ich);
	if (p) my_proto = dstrdup(p->text);
	p = find(HD_ARC, ich);
	if (p) my_arc = dstrdup(p->text);
	p = find(HD_SYS, ich);
	if (p) strupr(p->text);

	ich = del_header(HD_ARCERIN, ich);
	ich = del_header(HD_ARCEROUT, ich);

	ich = put_blk(ich, 2);
	fputs("\nBLK2 beendet,\nEigene Daten uebertragen:\n", deblogfile);
	wr_para(ich, deblogfile);

	alarm(30);
	blk3 = get_blk(3);
	alarm(0);
	if (auflegen_empfangen) return;
	fputs("\nBLK3 beendet,\nProtokoll und Arcer-Vroschlaege:\n", deblogfile);
	wr_para(blk3, deblogfile);

	/*
	 *  Daten fuer diese Verbindung ermitteln
	 */
	p = find(HD_PROTO, blk3);
	if (p) connection.proto = dstrdup(p->text);
	p = find(HD_ARCEROUT, blk3);
	if (p) connection.remote_arc = dstrdup(p->text);
	p = find(HD_ARCERIN, blk3);
	if (p) connection.local_arc = dstrdup(p->text);
	else connection.local_arc = dstrdup(connection.remote_arc);

	blk3_arc = dstrdup(connection.remote_arc);
	if (!ist_in(my_arc, blk3_arc))
		logoff("Daten-Fehler: Packer aus BLK3 sind nicht moeglich");
	blk3_proto = dstrdup(connection.proto);
	if (!ist_in(my_proto, blk3_proto))
		logoff("Daten-Fehler: Protokoll aus BLK3 ist nicht moeglich");
	blk4 = put_blk(NULL, 4);
	fputs("BLK4 gesendet:\n", deblogfile);
	if (blk4) {
		wr_para(blk4, deblogfile);
	} else {
		fputs("(leer)\n", deblogfile);
	}

	if (!ist_gast) {
		f = fopen(fname, "w");
		if (!f) {
			fputs("Kann aktuelle Daten nicht speichern...\n", deblogfile);
			return;
		}
		fputs("X-Call: ZCONNECT\n", f);
		anrufer = add_header(connection.proto, HD_PROTO, del_header(HD_PROTO, anrufer));
		anrufer = add_header(connection.remote_arc, HD_ARCERIN, del_header(HD_ARCERIN, anrufer));
		anrufer = add_header(connection.local_arc, HD_ARCEROUT, del_header(HD_ARCEROUT, anrufer));
		wr_para(anrufer, f);
		fclose(f);
	}

	dfree(blk3_arc);
	dfree(blk3_proto);
	free_para(ich);
	free_para(anrufer);
	free_para(blk3);
	if (blk4) free_para(blk4);
}

