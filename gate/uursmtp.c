/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 1999       Andreas Barth, Option "-p"
 *  Copyright (C) 1999       Matthias Andree, Option "-s"
 *  Copyright (C) 1996-2000  Dirk Meyer
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
 * uursmtp - liest einen SMTP-Batch und erzeugt eine ZCONNECT Datei
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "istring.h"
#include <ctype.h>
#include <errno.h>

#include "boxstat.h"
#include "ministat.h"
#include "utility.h"
#include "crc.h"
#include "header.h"
#include "hd_nam.h"
#include "hd_def.h"
#include "uulog.h"
#include "mime.h"
#include "lib.h"
#include "uuconv.h"
#include "gtools.h"
#include "sysexits2.h"

#ifdef APC_A2B
#include "apc_a2b.h"
#endif

int main_is_mail = 1;	/* Wir sind hauptsaechlich fuer PM EMP: zustaendig */
const char *hd_crlf = "\n";

typedef enum {	cmd_noop, cmd_helo, cmd_mail, cmd_rcpt, cmd_data,
		cmd_rset, cmd_quit, cmd_unknown } smtp_cmd;

typedef struct {
	const char *name;
	smtp_cmd cmd;
} cmd_list, *cmd_p;

typedef struct forward_st {
	char *path;
	struct forward_st *next;
} forward_list, *forward_p;

void convert(FILE *, FILE *);
void clear(void);
void enterfwd(const char *path);
void convdata(FILE *smtp, FILE *zconnect);

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

/* ist immer 2 * bufflen gross, bei APC_A2B 4 * buflen */
static char *bigbuffer	= NULL;
static char *smallbuffer= NULL;		/* wird dynamisch vergroessert */
static size_t bufflen = 0;		/* Groesse von smallbuffer */

static char *smtpdomain	= NULL;
static char *reverspath = NULL;
static forward_p fwdpaths = NULL;
extern char wab_name[], wab_host[], wab_domain[];

const char *fqdn = NULL;

void
usage(void)
{
	fputs(
"UUrsmtp  -  RFC821/822 Batch nach ZCONNECT konvertieren\n"
"Aufrufe:\n"
"        uursmtp (BSMTP-Datei) (ZCONNECT-Datei)\n"
"          alter Standard, Eingabedatei wird geloescht\n"
"        uursmtp -f (FQDN-ZCONNECT-Host)\n"
"          alter Standard, Eingabe von stdin,\n"
"          Ausgabedatei wird im Verzeichns des Systems erzeugt.\n"
"        uursmtp -d (BSMTP-Datei) (ZCONNECT-Datei)\n"
"          Modus mit hoechster Sicherheit, oder zum Testen.\n"
"        uursmtp -p [ (FQDN-ZCONNECT-Host) ]\n"
"          Echte Pipe\n"
"        uursmtp -s (FQDN-ZCONNECT-host) (Absender) (Empfaenger) [...]\n"
"          Einzelne Nachricht in der Pipe mit Envelope Daten\n"
, stderr);
	exit( EX_USAGE );
}

void
do_help(void)
{
	fputs(
"UUrsmtp  -  convert RFC821/822 BSMTP mail-batch to zconnect\n"
"usage:\n"
"        uursmtp BSMTP-file ZCONNECT-file\n"
"          old interface, BSMTP-file will be deleted\n"
"        uursmtp -f FQDN-ZCONNECT-host\n"
"          old interface, BSMTP input will be read from stdin\n"
"          outfile will be generated in the directory of the host.\n"
"        uursmtp -d BSMTP-file ZCONNECT-file\n"
"          secure mode, no delete, no directory search.\n"
"        uursmtp -p [ FQDN-ZCONNECT-host ]\n"
"        uursmtp --pipe\n"
"          full Pipe\n"
"        uursmtp -s [ FQDN-ZCONNECT-host ] Sender To [To ...]\n"
"        uursmtp --smtp [ FQDN-ZCONNECT-host ] Sender To [To ...]\n"
"          SMTP input from stdin, envelope in commandline\n"
"          outfile will be generated in the directory of the host.\n"
"        uursmtp --output ZCONNECT-file [ --remove ] BSMTP-file\n"
"          gnu standard\n"
"        uursmtp --version\n"
"          print version and copyright\n"
"        uursmtp --help\n"
"          print this text\n"
"\n"
"Report bugs to dirk.meyer@dinoex.sub.org\n"
, stderr);
	exit( EX_OK );
}

int
main(int argc, const char *const *argv)
{
	FILE *fin, *fout;
	const char *cptr;
	const char *name;
	const char *remove_me;
	const char *input_file;
	const char *output_file;
	char *dir_name;
	int ready;
	int smtp;
	char ch;

	initlog("uursmtp");
	minireadstat();
	srand( (unsigned) time(NULL));

	bufflen = 100000;
	smallbuffer = dalloc(bufflen);
	bigbuffer = dalloc(bufflen * 2);
	if (!bigbuffer || !smallbuffer) {
		fputs("Nicht genug Arbeitsspeicher!\n", stderr);
		exit( EX_TEMPFAIL );
	}

	name = argv[ 0 ];
	remove_me = NULL;
	input_file = NULL;
	output_file = NULL;
	dir_name = NULL;
	fin = NULL;
	fout = NULL;
	ready = 0;
	smtp = 0;

	/* check for commandline */
	while ( --argc > 0 ) {
		cptr = *(++argv);
		if ( *cptr == '-' ) {
			ch = *(++cptr);
			switch ( tolower( ch ) ) {
			case '-':
				cptr ++;
				if ( stricmp( cptr, "help" ) == 0 ) {
					do_help();
				};
				if ( stricmp( cptr, "version" ) == 0 ) {
					do_version( "UUrsmtp" );
				};
				if ( stricmp( cptr, "output" ) == 0 ) {
					if ( ready != 0 )
						usage();
					GET_NEXT_DATA( cptr );
					output_file = cptr;
					fqdn = NULL;
					ready ++;
					break;
				};
				if ( stricmp( cptr, "remove" ) == 0 ) {
					if ( ready != 0 )
						usage();
					GET_NEXT_DATA( cptr );
					input_file = cptr;
					remove_me = cptr;
					ready ++;
					break;
				};
				if ( stricmp( cptr, "pipe" ) == 0 ) {
					if ( ready != 0 )
						usage();
					input_file = "-";
					output_file = "-";
					ready ++;
					break;
				};
				if ( stricmp( cptr, "smtp" ) == 0 ) {
					if ( ready != 0 )
						usage();
					input_file = "-";
					/* Ein Argument ist Systenmane */
					GET_NEXT_DATA( cptr );
					fqdn = cptr;
					ready ++;
					smtp ++;
					break;
				};
				usage();
				break;
			case 'o':
				if ( ready != 0 )
					usage();
				GET_NEXT_DATA( cptr );
				output_file = cptr;
				fqdn = NULL;
				ready ++;
				break;
			case 'd':
				if ( ready != 0 )
					usage();
				/* Beide Argumente sind Dateien */
				GET_NEXT_DATA( cptr );
				input_file = cptr;
				GET_NEXT_DATA( cptr );
				output_file = cptr;
				ready ++;
				break;
			case 'p':
				if ( ready != 0 )
					usage();
				/* echte Pipe */
				input_file = "-";
				output_file = "-";
				ready ++;
				/* Ein Argument ist Systenmane */
				argv++; argc--; cptr = *argv;
				if ( cptr != NULL ) {
					/* Optional */
					fqdn = cptr;
				};
				break;
			case 'f':
				if ( ready != 0 )
					usage();
				input_file = "-";
				/* Ein Argument ist Systenmane */
				GET_NEXT_DATA( cptr );
				fqdn = cptr;
				ready ++;
				break;
			case 's':
				if ( ready != 0 )
					usage();
				input_file = "-";
				/* Ein Argument ist Systenmane */
				GET_NEXT_DATA( cptr );
				fqdn = cptr;
				ready ++;
				smtp ++;
				break;
			default:
				usage();
				break;
			};
			continue;
		};
		if ( smtp > 0 ) {
			if ( smtp == 1 ) {
				clear();
		   		smtpdomain = dstrdup( "localhost" );
				reverspath = dstrdup(cptr);
				smtp ++;
				continue;
			};
			enterfwd(cptr);
			continue;
		};
		/* erstes freies Argument */
		if ( input_file == NULL ) {
			/* Argument ist Eingabe-Datei */
			input_file = cptr;
			if ( ready == 0 )
				remove_me = cptr;
			continue;
		};
		/* zweites freies Argument */
		if ( ( fqdn == NULL )
		&& ( output_file == NULL ) ) {
			/* Argument ist Ausgabe-Datei */
			output_file = cptr;
			ready ++;
			continue;
		};
		/* weitere Argumente */
		usage();
	};
	if ( ready == 0 )
		usage();
	if ( input_file == NULL )
		usage();
	if ( strcmp( input_file, "-" ) == 0 ) {
		fin = stdin;
	} else {
		fin = fopen( input_file, "rb");
	};
	if ( fin == NULL ) {
		fprintf( stderr,
			"%s: error open input file %s: %s\n",
			name, input_file, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
	if ( fqdn != NULL ) {
#ifdef ENABLE_SPOOLDIR_SHORTNAME
		char *p, *p1;
#endif

		strcpy(datei, netcalldir);
		strcat(datei, "/");
		strcat(datei, fqdn);
#ifdef ENABLE_SPOOLDIR_SHORTNAME
		p = strrchr(datei, '/');
		if (p) {
			p1 = strchr(p+1, '.');
			if (p1) *p1 = '\0';
		}
#endif
		strcat(datei, "/");
		dir_name = dstrdup( datei );
		fout = open_new_file( name, dir_name, ".prv" );
		dfree( dir_name );
		output_file = datei;
	} else {
		if ( output_file == NULL )
			usage();
		if ( strcmp( output_file, "-" ) == 0 ) {
			fout = stdout;
		} else {
			fout = fopen( output_file, "ab");
		}
	};
	if ( fout == NULL ) {
		fprintf( stderr,
			"%s: error create output file %s: %s\n",
			name, output_file, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	if ( smtp > 0 ) {
		convdata(fin, fout);
	} else {
		while (!feof(fin))
			convert(fin, fout);
	};

	/* stdin/stdout nicht schiessen, das mach der parent */
	if ( fin != stdin )
		fclose(fin);
	if ( fout != stdout )
		fclose(fout);
	if ( remove_me != NULL ) {
		if ( remove(remove_me) ) {
			fprintf( stderr,
				"%s: error removing input file %s: %s\n",
				name, remove_me, strerror( errno ) );
			exit( EX_CANTCREAT );
		}
	};
	exit( EX_OK );
	return 0;
}

static char *
smtp_gets(char *line, size_t s, FILE *f)
{
	char *p;

	/* be sure, no overrun */
	if ( s > 32767 )
		s = 32767;
	if (!fgets(line, (int)s, f)) return NULL;
	p = strchr(line, '\n');
	if (p) {
		if (*(p-1) == '\r') p--;
		*p++ = '\n';
		*p = '\0';
	}
	return line;
}

void
convert(FILE *smtp, FILE *zconnect)
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
		   case cmd_data: convdata(smtp, zconnect);
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

void
clear(void)
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

void
enterfwd(const char *path)
{
	forward_p neu;

	neu = dalloc(sizeof(forward_list));
	neu->next = fwdpaths;
	fwdpaths = neu;
	neu->path = dstrdup(path);
}

void
convdata(FILE *smtp, FILE *zconnect)
{
	char *n;
	const char *habs, *mid;
	static char rna[MAXLINE];
	size_t msglen;
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
	habs = p ? p->text : "-";
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
		*n = '\0';
		/* Ende der Nachricht hat damit auf jeden Fall ein \0 */
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
		newlog(U2ZLOG, "mid=%s, from=%s, to=%s size=%ld",
			mid, habs, emp->path, (long)msglen );
	}
	if (!cnt) {
		deliver = 0;
		return;
	}

	splitaddr(reverspath, wab_name, wab_host, wab_domain, rna);

	/* Header konvertieren und ausgeben */
	hd = convheader(hd, zconnect, reverspath);

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
	     mime_info.encoding == cte_none ||	/* deren kodierung */
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

#ifndef DISABLE_UUCP_SERVER
	for (p=hd; p; p=p->next) {
	    header_p t;
	    for(;p;t=p, p=p->other) {
		char *x=decode_mime_string(p->text);
		iso2pc(x);
#ifndef ENABLE_U_X_HEADER
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

