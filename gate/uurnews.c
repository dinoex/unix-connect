/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 1999       Andreas Barth, Option "-p"
 *  Copyright (C) 2001       Detlef Würkner
 *  Copyright (C) 1996-2001  Dirk Meyer
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
 * uurnews - liest einen NEWS-Batch und erzeugt eine ZCONNECT Datei
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "istring.h"
#include <ctype.h>
#include <errno.h>

#include "utility.h"
#include "crc.h"
#include "header.h"
#include "uulog.h"
#include "hd_nam.h"
#include "hd_def.h"
#include "boxstat.h"
#include "ministat.h"
#include "mime.h"
#include "lib.h"
#include "uuconv.h"
#include "gtools.h"
#include "sysexits2.h"

#ifdef APC_A2B
#include "apc_a2b.h"
#endif

/*
 *  Globale Variable fuer die aktuelle Batch-Laenge
 *  (wird auf 0 heruntergezaehlt)
 */
long readlength = 0;

void convert(FILE *, FILE *);
void convdata(FILE *news, FILE *zconnect);


static char *bigbuffer	= NULL;
static char *readbuffer	= NULL;
size_t buffsize = 0;	/* gegenwaertig allokierte Buffer-Groesse */

const char *fqdn = NULL;
int main_is_mail = 0;	/* Nein, wir sind nicht fuer PM EMP:s zustaendig */

void
usage(void)
{
	fputs(
"UUrnews  -  RFC1036 Batch nach ZCONNECT konvertieren\n"
"Aufrufe:\n"
"        uurnews (RNEWS-Datei) (ZCONNECT-Datei)\n"
"          alter Standard, Eingabedatei wird geloescht\n"
"        uurnews -f (FQDN-ZCONNECT-Host)\n"
"          alter Standard, Eingabe von stdin,\n"
"          Ausgabedatei wird im Verzeichns des Systems erzeugt.\n"
"        uurnews -d (RNEWS-Datei) (ZCONNECT-Datei)\n"
"          Modus mit hoechster Sicherheit, oder zum Testen.\n"
"        uurnews -p [ (FQDN-ZCONNECT-Host) ]\n"
"          Echte Pipe\n"
, stderr);
	exit( EX_USAGE );
}

void
do_help(void)
{
	fputs(
"UUrnews  -  convert RFC1036 news batch to zconnect\n"
"usage:\n"
"        uurnews RNEWS-file ZCONNECT-file\n"
"          old interface, RNEWS-file will be deleted\n"
"        uurnews -f fqdn-zconnect-host\n"
"          old interface, RNEWS input will be read from stdin\n"
"          outfile will be generated in the directory of the host.\n"
"        uurnews -d RNEWS-file ZCONNECT-file\n"
"          secure mode, no delete, no directory search.\n"
"        uurnews -p [ fqdn-zconnect-host ]\n"
"        uurnews --pipe\n"
"          full Pipe\n"
"        uurnews --output ZCONNECT-file [ --remove ] RNEWS-file\n"
"          gnu standard\n"
"        uurnews --version\n"
"          print version and copyright\n"
"        uurnews --help\n"
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
	char ch;

	initlog("uurnews");
	minireadstat();
	srand( (unsigned) time(NULL));

	name = argv[ 0 ];
	remove_me = NULL;
	input_file = NULL;
	output_file = NULL;
	dir_name = NULL;
	fin = NULL;
	fout = NULL;
	ready = 0;

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
					do_version( "UUrnews" );
				};
				if ( stricmp( cptr, "output" ) == 0 ) {
					if ( ready != 0 )
						usage();
					GET_NEXT_DATA( cptr );
					output_file = cptr;
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
				usage();
				break;
			case 'o':
				if ( ready != 0 )
					usage();
				GET_NEXT_DATA( cptr );
				output_file = cptr;
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
			default:
				usage();
				break;
			};
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
		fout = open_new_file( name, dir_name, ".brt" );
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

	while (!feof(fin))
		convert(fin, fout);

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


void
convert(FILE *news, FILE *zconnect)
{
	static char line[MAXLINE];

	while (!feof(news)) {
		 if (fgets(line, MAXLINE, news) == NULL) break;
		 readlength = -1;
		 sscanf(line, "#! rnews %ld", &readlength);
		 if (readlength < 1) {
		 	fprintf(stderr,
				"\n\nABBRUCH: Batch-Laenge \"%s\"\n", line);
			exit( EX_DATAERR );
		 }
		 convdata(news, zconnect);
	}
}

void
convdata(FILE *news, FILE *zconnect)
{
	char *n, *s;
	size_t msglen, wrlen;
	header_p hd, p, t;
	int binaer, eightbit;
	mime_header_info_struct mime_info;
	int ismime, decoded;

	msglen = 0;

	hd = uu_rd_para(news);

	/* MIME-Headerzeilen lesen */
	ismime = parse_mime_header(0, hd, &mime_info);

	if (readlength >= (long)buffsize) {
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
		msglen = fread(readbuffer, 1, (size_t)readlength, news);
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
	hd = convheader(hd, zconnect, NULL);

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
	hd = del_header(HD_UU_XREF, hd);
	for (p=hd; p; p=p->next) {
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
#ifdef DISABLE_FULL_GATE
/* Artikel, die von Usenet-Seite aus hereinkommen, duerfen nicht wieder
   herausgeschickt werden. */
	fprintf(zconnect, HN_X_DONT_GATE_IT": \r\n");
#endif
	free_para(hd);

	/* Jetzt sind alle Header-Angelegenheiten erledigt bis
	 * auf FILE:, LEN:, TYP:, CHARSET: u. dgl.
	 * Das ist fuer Mail und News ziemlich gleich,
	 * es steht daher in uuconv.c
	 */

	make_body(bigbuffer, (size_t)msglen, &mime_info, binaer,
		readbuffer, zconnect);

	if (mime_info.charsetname)
		free(mime_info.charsetname);
	if (mime_info.filename)
		free(mime_info.filename);
}

