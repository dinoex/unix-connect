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
 * uursmtp - liest einen SMTP-Batch und erzeugt eine ZCONNECT Datei
 *
 */

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
#ifdef HAS_BSD_DIRECT
#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "ministat.h"
#include "utility.h"
#include "header.h"
#include "hd_nam.h"
#include "hd_def.h"
#include "version.h"
#include "uulog.h"
/* #include "trap.h" */
#include "mime.h"
#include "uuconv.h"

#ifdef APC_A2B
#include "apc_a2b.h"
#endif

int main_is_mail = 1;	/* Wir sind hauptsaechlich fuer PM EMP: zustaendig */
char *hd_crlf = "\n";

header_p smtp_rd_para(FILE *);

typedef enum {	cmd_noop, cmd_helo, cmd_mail, cmd_rcpt, cmd_data,
		cmd_rset, cmd_quit, cmd_unknown } smtp_cmd;

typedef struct {
	char *name;
	smtp_cmd cmd;
} cmd_list, *cmd_p;

typedef struct forward_st {
	char *path;
	struct forward_st *next;
} forward_list, *forward_p;

void usage(void);
void convert(FILE *, FILE *);
void clear(void);
void enterfwd(char *path);
void convdata(char *smtpdomain, char *reverspath, forward_p fwdpaths,
	FILE *smtp, FILE *zconnect);

static int deliver;

static cmd_list cmds[] = {
	{ "NOOP", cmd_noop },
	{ "HELO", cmd_helo },
	{ "MAIL", cmd_mail },
	{ "RCPT", cmd_rcpt },
	{ "DATA", cmd_data },
	{ "RSET", cmd_rset },
	{ "QUIT", cmd_quit },
	{ NULL, cmd_unknown }
};
	
static char *bigbuffer	= NULL;		/* ist immer 2 * bufflen gross, bei APC_A2B 4 * buflen */
static char *smallbuffer= NULL;		/* wird dynamisch vergroessert */
static size_t bufflen = 0;		/* Groesse von smallbuffer */
static char *smtpdomain	= NULL;
static char *reverspath = NULL;
static forward_p fwdpaths = NULL;
extern char *netcalldir;
extern char wab_name[], wab_host[], wab_domain[];

char *fqdn = NULL;


int main(int argc, char **argv)
{
	FILE *fin, *fout;
	char datei[2048];
	int filter;

/*	init_trap(argv[0]); */
	filter = 0;
	ulibinit();	
	minireadstat();
	srand(time(NULL));
	if (argc != 3) usage();
	bufflen = 100000;
	smallbuffer = dalloc(bufflen);
	bigbuffer = dalloc(bufflen * 2);
	if (strcmp(argv[1], "-p") == 0) {
		fin = stdin;
		fout= stdout;
	} else
	if (strcmp(argv[1], "-f") == 0) {
		time_t j;
#ifdef SPOOLDIR_SHORTNAME
		char *p, *p1;
#endif
		char tmp[20];
		char *tmp2;
		int fh;

		fin = stdin;
		fqdn = argv[2];
		filter = 1;

		j = time(NULL);
		strcpy(datei, netcalldir);
		strcat(datei, "/");
		strcat(datei, fqdn);
#ifdef SPOOLDIR_SHORTNAME
		p = strrchr(datei, '/');		/* Strip Domain */
		if (p) {				/* vom Dir-Namen */
			p1 = strchr(p+1, '.');		/* (siehe smail.transport) */
			if (p1) *p1 = '\0';
		}
#endif
		strcat(datei, "/");
		tmp2 = dstrdup(datei);
		sprintf(tmp, "%08lx.prv", (long)j);
		strcat(datei, tmp);
/* Um sicherzugehen, dass wir keine Datei ueberschreiben,
 * oeffnen wir sie zunaechst mit O_EXCL. Das geht aber nur
 * mit open, waehrend wir unten FILE* verwenden. Also
 * muessen wir diese seltsame Konstruktion machen: */
		fh = open(datei,O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP);
		while(fh<0)
		{
			((long)j)++;
			sprintf(tmp,"%08lx.prv",(long)j);
			strcpy(datei,tmp2);
			strcat(datei,tmp);
			fh=open(datei,O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP);
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
	fputs(	"uursmtp  -  RFC821/822  Batch nach ZCONNECT konvertieren\n"
		"Aufruf:\n"
		"        uursmtp (SMTP-Datei) (ZCONNECT-Datei)\n"
		"oder    uursmtp -f (FQDN-ZCONNECT-Host)\n"
		"          (SMTP wird von stdin gelesen)\n"
		"          (ZCONNECT in das entsprechende Netcall-Dir)\n"
		"oder    uurnews -p (FQDN-ZCONNECT-Host)\n"
		"           (SMTP wird von stdin gelesen)\n"
		"           (ZCONNECT nach stdout)\n"
		, stderr);
	exit(1);
}

static char *smtp_gets(char *line, size_t s, FILE *f)
{
	char *p;
	
	if (!fgets(line, s, f)) return NULL;
	p = strchr(line, '\n');
	if (p) {
		if (*(p-1) == '\r') p--;
		*p++ = '\n';
		*p = '\0';
	}
	return line;
}

void convert(FILE *smtp, FILE *zconnect)
{
	static char line[MAXLINE];
	cmd_p p;
	int stop;
	char *s, *q;

	clear();
	stop = 0;	
	while (!stop && !feof(smtp)) {
		/*
		 * Oberste Ebene: SMTP Kommando-Parser
		 * Minimal-Implementierung nach RFC821
		 */
		 if (!smtp_gets(line, MAXLINE, smtp)) break;
		 s = strchr(line, '\n');
		 if (s) *s = '\0';
		 line[4] = '\0';	/* SMTP Kommandos sind 4 Zeichen */
		 strupr(line);
		 for (p=cmds; p->name; p++)
		 	if (strcmp(p->name, line) == 0) break;
		 if (p->name) switch(p->cmd) {
		   case cmd_noop:
		   		  break;
		   case cmd_helo: clear();
		   		  smtpdomain = dstrdup(line+5);
		   		  break;
		   case cmd_mail: if (reverspath) dfree(reverspath);
		   		  s = strchr(line+5, '>');
		   		  if (s) *s = '\0';
		   		  s = strchr(line+5, '<');
		   		  if (!s) s = line+4;
		   		  q = strrchr(s+1, ':');
		   		  reverspath = dstrdup(q ? q+1 : s+1);
		   		  break;
		   case cmd_rcpt: s = strchr(line+5, '>');
		   		  if (s) *s = '\0';
		   		  s = strchr(line+5, '<');
		   		  if (!s) s = line+4;
		   		  q = strrchr(s+1, ':');
		   		  enterfwd(q ? q+1 : s+1);
		   		  break;
		   case cmd_data: convdata(smtpdomain, reverspath, fwdpaths, smtp, zconnect);
		   		  clear();
		   		  break;
		   case cmd_rset: clear();
		   		  break;
		   case cmd_quit: stop = 1;
		   		  break;
		   case cmd_unknown:
		   default:
		   		  break;
		 }
	}
}

void clear(void)
{
	forward_p p, n;
	
	if (reverspath) dfree(reverspath);
	reverspath = NULL;
	for (n=NULL, p=fwdpaths; p; p=n) {
		n = p->next;
		dfree(p->path);
		dfree(p);
	}
	fwdpaths = NULL;
	reverspath = NULL;
}

void enterfwd(char *path)
{
	forward_p neu;

	neu = dalloc(sizeof(forward_list));
	neu->next = fwdpaths;
	fwdpaths = neu;
	neu->path = dstrdup(path);
}

void convdata(char *smtpdomain, char *reverspath, forward_p fwdpaths,
	FILE *smtp, FILE *zconnect)
{
	char *n, *abs, *mid;
	static char rna[MAXLINE];
	long msglen;
	header_p hd, p;
	forward_p emp;
	int binaer, cnt;
	int eightbit;
	mime_header_info_struct mime_info;
	int ismime, decoded;
	time_t now;

	/* Zuerst den Header lesen: */
	hd = smtp_rd_para(smtp);

	/* MIME-Headerzeilen lesen. */
	ismime = parse_mime_header(0, hd, &mime_info);
	
	/* Daten fuer das Logfile zusammenstellen: */
	p = find(HD_UU_FROM, hd);
	abs = p ? p->text : "-";
	p = find(HD_UU_MESSAGE_ID, hd);
	mid = p ? p->text : "-";

	/* Inhalt lesen: */
	msglen = 0;
	n = bigbuffer;
	while (!feof(smtp)) {
		if (smtp_gets(n, bufflen, smtp) == NULL)
			break;
		if ((msglen+4) > bufflen) {
			bufflen *= 2;
			dfree(smallbuffer);
			smallbuffer = bigbuffer;
			bigbuffer = dalloc(bufflen *
#ifdef APC_A2B
			/* Fuer die A2B-Wandlung brauchen wir evtl.
				so viel Platz... */
							4
#else
							2
#endif
							  );
 
			memcpy(bigbuffer, smallbuffer, msglen + strlen(n)+1);
			n = bigbuffer + msglen;
		}
		if (strcmp(".\n", n) == 0)
			break;  /* Ende der Nachricht */
		if (*n == '.') strcpy(n, n+1);
		while (*n) {
			if (*n == '\r' || *n == '\n') break;
			n++;
			msglen++;
		}
		*n++ = '\r';
		*n++ = '\n';
		*n = '\0';    /* Ende der Nachricht hat damit auf jeden Fall ein 0 */
		msglen += 2;
		/*
		 *  Invariant: bigbuffer enthaelt die Nachricht der Laenge msglen,
		 *  die aber auch komplett in smallbuffer passt.
		 */
	}

	deliver = 1;

	now = time(NULL);
	localtime(&now);
	/* Envelope-Adresse konvertieren: */
	for(cnt=0,emp=fwdpaths; emp; emp=emp->next) {
		cnt += convaddr(HN_EMP, emp->path, 1, zconnect);
		logfile(U2ZLOG, abs, emp->path, mid, "\n");
	}
	if (!cnt) {
		deliver = 0;
		return;
	}

	splitaddr(reverspath, wab_name, wab_host, wab_domain, rna);
	
	/* Header konvertieren und ausgeben: */
	hd = convheader(hd, zconnect, smtpdomain, reverspath);

	/* Mimes Content-transfer-encoding decodieren - danach haben wir,
	 * abhaengig vom Content-Type, evtl. Binaerdaten.
	 */
	decoded = decode_cte(bigbuffer, &msglen, &eightbit, &mime_info);

	/* decoded != 0 heisst jetzt, wir haben decodierte Daten.
	 * Wenn ja, Content-Transfer-Encoding loeschen, denn das
	 * stimmt nicht mehr.
	 */

	if (decoded)
		hd = del_header(HD_UU_CONTENT_TRANSFER_ENCODING, hd);

	/* eightbit != 0 heisst, wir haben auch tatsaechlich 8-Bit-Daten.
	 * Eine Nachricht behandeln wir dann als binaer, wenn sie
	 * a) eine Mime-Nachricht ist, sie b) gerade eben tatsaechlich deco-
	 * diert wurde und sie c) keinen Text enthaelt.
	 */
	if (ismime && decoded && mime_info.type != cty_text && mime_info.type != cty_none)

		binaer = 1;
	else
		binaer = 0;

	/* MIME-Headerzeilen durchreichen? Eine schwere Frage.
	 * Wir entscheiden uns so:
	 */

	if (ismime && (				/* MIME-Nachricht */
	     mime_info.encoding == cte_none ||  /* deren kodierung */
	     decoded				/* aufgeloest wurde */
	   )
	   && (
	     mime_info.unixconnect_multipart || /* und die text/plain */
	     mime_info.text_plain		/* oder spezial-Multi- */
	   )					/* part ist */
	 ) {
		hd = del_header(HD_UU_MIME_VERSION, hd);
		hd = del_header(HD_UU_CONTENT_TYPE, hd);
		hd = del_header(HD_UU_CONTENT_TRANSFER_ENCODING, hd);
	}

#ifdef UUCP_SERVER
	for (p=hd; p; p=p->next) {
	    header_p t;
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
	free_para(hd);

	/* Jetzt sind alle Header-Angelegenheiten erledigt bis
	 * auf FILE:, LEN:, TYP:, CHARSET: u. dgl.
	 * Das ist fuer Mail und News ziemlich gleich,
	 * es steht daher in uuconv.c
	 */

	make_body(bigbuffer, msglen, &mime_info, binaer, 
		smallbuffer, zconnect);

	if (mime_info.filename)
		free(mime_info.filename);
}
