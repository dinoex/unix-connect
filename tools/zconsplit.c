/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 *   zconsplit.c:
 *
 *      Liest einen ZCONNECT-Puffer ein und erzeugt daraus zwei:
 *        einen *.prv mit alle PM's sowie
 *        einen *.brt mit allen oeffentlichen Nachrichten.
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "boxstat.h"
#include "ministat.h"
#include "utility.h"


static char *buffer;		/* Nachrichten-Buffer */
static size_t buf_alloc;	/* Groesse des bisher allokierten Buffer's */

void convert(FILE *, FILE *, FILE *);

const char *hd_crlf = "\r\n";

static void
usage(void)
{
	fputs("Aufruf:\tzconsplit (ZCONNECT-Datei)\n"
	      "\t\tum die angegebene ZCONNECT-Datei in zwei\n"
	      "\t\taufzuspalten (.prv und .brt).\n", stderr);
	exit(1);
}

int
main(int argc, char **argv)
{
	FILE *inp, *prv, *brt;
	char name[FILENAME_MAX];

	minireadstat();

	if (argc != 2) usage();
	inp = fopen(argv[1], "rb");
	if (!inp) {
		perror(argv[1]);
		usage();
	}
	strcpy(name, argv[1]);
	strcat(name, ".prv");
	prv = fopen(name, "ab");
	if (!prv) {
		perror(name);
		usage();
	}
	strcpy(name, argv[1]);
	strcat(name, ".brt");
	brt = fopen(name, "ab");
	if (!brt) {
		perror(name);
		usage();
	}
	set_rd_para_reaction(0);
	buffer = NULL;
	buf_alloc = 0;
	while (!feof(inp))
		convert(inp, prv, brt);
	fclose(inp);
	fclose(prv);
	fclose(brt);
	remove(argv[1]);

	return 0;
}

static header_p
dofind(int code, header_p h)
{
	h = find(code, h);
	if (!h) {
		fputs("FATAL: illegale Netcall-Datei\n", stderr);
		exit(5);
	}

	return h;
}

/*
 *  Lese eine Message aus *zcon, schreibe sie nach *prv oder *brt.
 */
void
convert(FILE *zcon, FILE *prv, FILE *brt)
{
	header_p hd, h, private, news;
	char *p;
	size_t mlen;

	/*
	 *  Kopf einlesen
	 */
	hd = rd_para(zcon);
	if (!hd) return;	/* eof: fertig */
	/*
	 *  Laenge ermittlen
	 */
	h = dofind(HD_LEN, hd);
	mlen = (size_t)atol( h->text );
	/*
	 *  Gegebenenfalls buffer vergroessern
	 */
	if (mlen > buf_alloc) {
		dfree(buffer);
		buf_alloc = mlen;
		buffer = malloc(buf_alloc);
		if (!buffer) {
			fprintf(stderr,
				"FATAL: Speichermangel: %ld\n", (long)mlen);
			exit(5);
		}
	}
	/*
	 *  Inhalt einlesen
	 */
	fread(buffer, mlen, 1, zcon);
	/*
	 *  Nachricht ist komplett im Speicher, jetzt aufteilen:
	 */
	h = dofind(HD_EMP, hd);
	news = private = NULL;
	while (h) {
		/*
		 *  Alle EMP: durchsehen und eine Kopie in *private oder *news
		 *  einbauen, jeweils ein KOP: in den anderen.
		 *
		 *  ACHTUNG: entgegen der ZCONNECT Definition wird hier durch
		 *           das erste Zeichen '/' entschieden, ob es ein
		 *           Brett ist oder nicht. Im RFC gibt es keine direkt-
		 *           zugestellten Bretter...
		 */
		 if (*(h->text) == '/') {
		 	p = strchr(h->text, '@');
		 	if (p) *p = '\0';
		 	private = add_header(h->text, HD_KOP, private);
		 	news = add_header(h->text, HD_EMP, news);
		 } else {
		 	private = add_header(h->text, HD_EMP, private);
		 	news = add_header(h->text, HD_KOP, news);
		 }
		 /*
		  *  ok, naechster Empfaenger...
		  */
		 h = h->other;
	}
	/*
	 *  EMP: loeschen, den haben wir abgearbeitet.
	 */
	hd = del_header(HD_EMP, hd);
	/*
	 *  Ist ein news-Emp: dabei herausgekommen?
	 */
	h = find(HD_EMP, news);
	if (!h) {
		free_para(news);
		news = NULL;
	}
	/*
	 *  Gibt es einen private-EMP: ?
	 */
	h = find(HD_EMP, private);
	if (!h) {
		free_para(private);
		private = NULL;
	}
	/*
	 *  Nachricht(en) ausgeben:
	 *   1)  private bzw. news enthalten EMP: und KOP:
	 *   2)  hd enthaelt alle uebrigen Header
	 *   3)  buffer ist der Inhalt (mit Laenge mlen)
	 */
	if (private) {
		wr_para_continue(private, prv);
		wr_para(hd, prv);
		fwrite(buffer, mlen, 1, prv);
	}
	if (news) {
		wr_para_continue(news, brt);
		wr_para(hd, brt);
		fwrite(buffer, mlen, 1, brt);
	}
}
