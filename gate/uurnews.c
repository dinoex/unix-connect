/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1996-99  Dirk Meyer
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
 * uurnews - liest einen NEWS-Batch und erzeugt eine ZCONNECT Datei
 *
 */

/*
 * Dirk Meyer, 08.05.96
 * Das ueberfluessige '\r' nach "end" wird nicht mehr an
 * uudecode uebergeben. Die Meldung "uudecode: missing end"
 * verschwindet.
 * ist in uursmtp.c bereits eingebaut.
 *
 */
#define NO_RETURN_AFTER_END

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include "lib.h"
#ifdef NEED_VALUES_H
#include <values.h>
#endif
#include <errno.h>
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#ifdef HAS_BSD_DIRECT
#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utility.h"
#include "header.h"
#include "uulog.h"
#include "hd_nam.h"
#include "hd_def.h"
#include "version.h"
#include "ministat.h"
#include "mime.h"
/* #include "trap.h" */
#include "uuconv.h"

#ifdef APC_A2B
#include "apc_a2b.h"
#endif

header_p uu_rd_para(FILE *);

extern int dont_gate;

/*
 *  Globale Variable fuer die aktuelle Batch-Laenge
 *  (wird auf 0 heruntergezaehlt)
 */
long readlength = 0;

void usage(void);
void convert(FILE *, FILE *);
void convdata(FILE *news, FILE *zconnect);


static char *bigbuffer	= NULL;
static char *readbuffer	= NULL;
long buffsize = 0;	/* gegenwaertig allokierte Buffer-Groesse */

extern char *netcalldir;

char *fqdn = NULL;
int main_is_mail = 0;	/* Nein, wir sind nicht fuer PM EMP:s zustaendig */

int main(int argc, char **argv)
{
	FILE *fin, *fout;
	int filter;
	char datei[2000];

/*	init_trap(argv[0]); */
	filter = 0;
	ulibinit();
	minireadstat();
	srand(time(NULL));
	if (argc != 3) usage();
	if (strcmp(argv[1], "-p") == 0) {
		fin = stdin;
		fout= stdout;
	} else
	if (strcmp(argv[1], "-f") == 0) {
		time_t j;
		char tmp[20];
		char *tmp2;
		int fh;
#ifdef SPOOLDIR_SHORTNAME
		char *p, *p1;
#endif

		filter = 1;
		fin = stdin;
		fqdn = argv[2];

		j = time(NULL);
		sprintf(tmp, "%08lx.brt", (long)j);
		strcpy(datei, netcalldir);
		strcat(datei, "/");
		strcat(datei, fqdn);
#ifdef SPOOLDIR_SHORTNAME
		p = strrchr(datei, '/');
		if (p) {
			p1 = strchr(p+1, '.');
			if (p1) *p1 = '\0';
		}
#endif
		strcat(datei, "/");
		tmp2 = dstrdup(datei);
		strcat(datei, tmp);
/* Um sicherzugehen, dass wr keine Datei ueberschreiben,
 * oeffnen wir sie zunaechst mit O_EXCL. Um weiter unten
 * einen FILE* zu haben, schliessen wir sie anschliessend
 * wieder und oeffnen sie mit fopen() erneut. Potentielles
 * Sicherheitsproblem. */
		fh = open(datei,O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP);
		while(fh<0)
		{
			((long)j)++;
			sprintf(tmp,"%08lx.brt",(long)j);
			strcpy(datei,tmp2);
			strcat(datei,tmp);
			fh=open(datei,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR|S_IRGRP);
		}
		close(fh);
		fout = fopen(datei, "ab");
		free(tmp2);
	} else {
		fin = fopen(argv[1], "rb");
		fout = fopen(argv[2], "ab");
		strcpy(datei, argv[2]);
	}
	if (!fin) {
		perror(argv[1]);
		exit(1);
	}
	if (!fout) {
		perror(datei);
		exit(1);
	}
	convert(fin, fout);
	fclose(fout);

	if (!filter) {
		fclose(fin);
		remove(argv[1]);
	}

	return 0;
}

void usage(void)
{
	fputs(	"UUrnews  -  RFC1036 Batch nach ZCONNECT konvertieren\n"
		"Aufruf:\n"
		"        uurnews (NEWS-Datei) (ZCONNECT-Datei)\n"
		"oder    uurnews -f (FQDN-ZCONNECT-Host)\n"
		"           (News Batch von stdin)\n"
		"           (ZCONNECT in das entsprechende Netcall-Dir)\n"
		"oder    uurnews -p (FQDN-ZCONNECT-Host)\n"
		"           (News Batch von stdin)\n"
		"           (ZCONNECT nach stdout)\n"
		, stderr);
	exit(1);
}

void convert(FILE *news, FILE *zconnect)
{
	static char line[MAXLINE];

	while (!feof(news)) {
		 if (fgets(line, MAXLINE, news) == NULL) break;
		 readlength = -1;
		 sscanf(line, "#! rnews %ld", &readlength);
		 if (readlength < 1) {
		 	fprintf(stderr, "\n\nABBRUCH: Batch-Laenge \"%s\"\n", line);
			exit(1);
		 }
		 convdata(news, zconnect);
	}
}

void convdata(FILE *news, FILE *zconnect)
{
	char *n, *s;
	long msglen, wrlen;
	header_p hd, p, t;
	int binaer, eightbit;
	mime_header_info_struct mime_info;
	int ismime, decoded;

	msglen = 0;

	hd = uu_rd_para(news);

	/* MIME-Headerzeilen lesen */
	ismime = parse_mime_header(0, hd, &mime_info);

	if (readlength >= buffsize) {
		dfree(bigbuffer);
		dfree(readbuffer);
		buffsize = readlength+1000;
		bigbuffer = dalloc(buffsize*
#ifdef APC_A2B
					     4
#else
					     2
#endif
						);

		readbuffer = dalloc(buffsize);
	}

	/* Text lesen, LF nach CR/LF wandeln... */
	if (readlength > 0) {
		msglen = fread(readbuffer, 1, readlength, news);
		for (wrlen=0, s=readbuffer, n=bigbuffer; msglen; msglen--) {
			if (*s == '\n') {
				wrlen++;
				*n++ = '\r';
			}
			*n++ = *s++;
			wrlen++;
		}
	} else
		wrlen = 0;
	msglen=wrlen;

	/* Header konvertieren und ausgeben */
	hd = convheader(hd, zconnect, NULL, NULL);

	/* MIME Content-Transfer-Encoding decodieren */
	decoded = decode_cte(bigbuffer, &msglen, &eightbit, &mime_info);

	/* Wenn decodiert wurde, diese Headerzeile loeschen */
	if (decoded)
		hd = del_header(HD_UU_CONTENT_TRANSFER_ENCODING, hd);

	/* eightbit != 0 heisst, wir haben auch tatsaechlich-Daten.
	 * Eine Nachricht behandeln wir dann als binaer, wenn sie
	 * a) eine Mime-Nachricht ist, sie b) gerade eben tatsaechlich deco-
	 * diert wurde und sie c) keinen Text enthaelt.
	 */

	if (ismime && decoded && mime_info.type != cty_text && mime_info.type != cty_none)
		binaer=1;
	else
		binaer=0;

        /* MIME-Headerzeilen durchreichen? Eine schwere Frage.
         * Wir entscheiden uns so:
         */

        if (ismime && (                         /* MIME-Nachricht */
             mime_info.encoding == cte_none ||  /* deren kodierung */
             decoded                            /* aufgeloest wurde */
           )
           && (
             mime_info.unixconnect_multipart || /* und die text/plain */
             mime_info.text_plain               /* oder spezial-Multi- */
           )                                    /* part ist */
         ) {
                hd = del_header(HD_UU_MIME_VERSION, hd);
                hd = del_header(HD_UU_CONTENT_TYPE, hd);
                hd = del_header(HD_UU_CONTENT_TRANSFER_ENCODING, hd);
        }

#ifdef UUCP_SERVER
	hd = del_header(HD_UU_XREF, hd);
	if (!dont_gate)
	   for (p=hd; p; p=p->next) {
	     for(;p;t=p, p=p->other) {
	        char *x=decode_mime_string(p->text);
		to_pc(x);
#ifndef NO_PLUS_KEEP_X_HEADER
		fprintf (zconnect, "U-%s: %s\r\n", p->header, x);
#else
		/* TetiSoft: X- bleibt X- (Gatebau '97) */
		if (strncasecmp(p->header, "X-", 2) == 0)
			fprintf (zconnect, "%s: %s\r\n", p->header, x);
		else
			fprintf (zconnect, "U-%s: %s\r\n", p->header, x);
#endif
		dfree(x);
	      }
	     p=t;
	   }
#endif
#ifdef REAL_GATE
/* Artikel, die von Usenet-Seite aus hereinkommen, dürfen nicht wieder
   herausgeschickt werden. */
	fprintf(zconnect, HN_X_DONT_GATE_IT": \r\n");
#endif
	free_para(hd);
        /* Jetzt sind alle Header-Angelegenheiten erledigt bis
         * auf FILE:, LEN:, TYP:, CHARSET: u. dgl.
         * Das ist fuer Mail und News ziemlich gleich,
         * es steht daher in uuconv.c
         */

        make_body(bigbuffer, msglen, &mime_info, binaer,
                readbuffer, zconnect);

        if (mime_info.filename)
                free(mime_info.filename);

}

