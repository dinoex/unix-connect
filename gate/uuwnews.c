/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1999     Andreas Barth, Option "-p"
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
 * uuwnews - erzeugt einen RNEWS-Batch aus einer ZCONNECT-Datei
 *
 */

#include "config.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif
#include <ctype.h>
#ifdef NEED_VALUES_H
#include <values.h>
#endif
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sysexits.h>
#include <errno.h>

#include "lib.h"
#include "boxstat.h"
#include "ministat.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "uuapprove.h"
#include "uulog.h"
#include "mime.h"
#include "zconv.h"
#include "gtools.h"

int main_is_mail = 0;	/* Nein, wir erzeugen keine Mails */
const char eol[] = "\n";

void convert(FILE *, FILE *);

/* Wenn es ein Point ist: die Liste der erlaubten Absender */
header_p pointuser;
char *pointsys;		/* Name des Point-Systems (fuer die Message-Id) */

static char *bigbuffer	= NULL;
static char *smallbuffer= NULL;
#ifndef USE_ISO_IN_NEWS
extern char umlautstr[], convertstr[];
#endif

void usage(void)
{
	fputs(
"UUwnews  -  RFC1036  Batch aus ZCONNECT erzeugen\n"
"Aufrufe:\n"
"        uuwnews (ZCONNECT-Datei) (NEWS-Directory) [Absende-System]\n"
"          alter Standard, Eingabedatei wird gelöscht,\n"
"          Ausgabedatei wird im Verzeichns erzeugt.\n"
"        uuwnews -f (ZCONNECT-Datei) [Absende-System]\n"
"          Ergebnis geht nach stdout, Eingabedatei wird gelöscht,\n"
"        uuwnews -d (ZCONNECT-Datei) (RNEWS-Datei) [Absende-System]\n"
"          Modus mit höchster Sicherheit, oder zum Testen.\n"
"        uuwnews -p [Absendesystem]\n"
"          Echte Pipe\n"
, stderr);
	exit( EX_USAGE );
}

void do_help(void)
{
	fputs(
"UUwnews  -  convert from zconnect to RFC1036 news batch\n"
"usage:\n"
"        uuwnews ZCONNECT-file RNEWS-dir [ fqdn-zconnect-host ]\n"
"          old interface, ZCONNECT-file will be deleted\n"
"          output-file will be created in the specified directory\n"
"        uuwnews -f ZCONNECT-file [ fqdn-zconnect-host ]\n"
"          old interface, NEWS output will be written to stdout\n"
"        uuwnews -d ZCONNECT-file RNEWS-file [ fqdn-zconnect-host ]\n"
"          secure mode, no delete, no directory search.\n"
"        uuwnews -p [ fqdn-zconnect-host ]\n"
"        uuwnews --pipe [ fqdn-zconnect-host ]\n"
"          full Pipe\n"
"        uuwnews --output RNEWS-file [ --remove ] ZCONNECT-file \n"
"                [ fqdn-zconnect-host ]\n"
"          gnu standard\n"
"        uuwnews --version\n"
"          print version and copyright\n"
"        uuwnews --help\n"
"          print this text\n"
"\n"
"Report bugs to dirk.meyer@dinoex.sub.org\n"
, stderr);
	exit( EX_OK );
}

int main(int argc, const char *const *argv)
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

	initlog("uuwnews");
	pointuser = NULL;
	pointsys = NULL;
	ulibinit();
	minireadstat();
	srand( (unsigned) time(NULL));

	bigbuffer = malloc(BIGBUFFER);
	smallbuffer = malloc(SMALLBUFFER);
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
					do_version( "UUwnews" );
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
				break;
			case 'f':
				if ( ready != 0 )
					usage();
				/* Ein Argument ist Eingabe-Datei */
				GET_NEXT_DATA( cptr );
				input_file = cptr;
				remove_me = cptr;
				/* Ergebnis nach stdout */
				output_file = "-";
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
		if ( ( output_file == NULL )
		&& ( dir_name == NULL ) ) {
			/* Ergebnis im angegebene Verzeichnis */
			dir_name = dstrdup( cptr );
			ready ++;
			continue;
		};
		/* dittes freies Argument */
		if ( pointsys == NULL ) {
			FILE *f;
			char buf[FILENAME_MAX];

			pointsys = dstrdup( cptr );
			strlwr(pointsys);
			sprintf(buf, "%s/%s", systemedir, pointsys);
			f = fopen(buf, "r");
			if (f) {
				header_p sys, p;

				sys = rd_para(f);
				if (sys) {
					p = find(HD_POINT_USER, sys);
					pointuser = p;
				}
				fclose(f);
			};
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
	if ( dir_name != NULL ) {
		strcpy(datei, dir_name);
		strcat(datei, "/");
		dfree( dir_name );
		dir_name = dstrdup( datei );
		fout = open_new_file( name, dir_name, NULL );
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
	}
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

#define ENC(c)	(!((c) & 0x03f) ? '`' : (((c) & 0x03f) + ' '))

void convert(FILE *zconnect, FILE *news)
{
	header_p hd, p;
#ifdef NO_Z38_CROSS_ROUTING
	header_p pst, habs;
#endif
	FILE *tmp, *tmphd;
	time_t j;
	char id[30], zbuf[4], lines_line[50], *sp, *zp,
		*bufende, *c, *typ, *file;
	static int binno = 0;
	size_t bytecount, znr;
	int do_approve, multipart;
	int skip, has_lines;
#ifdef UUENCODE_CHKSUM
	int sum;
#endif
	size_t comment, ascii_len, len, lines;
	size_t buffree;
	int charset, wasmime, eightbit;
	mime_header_info_struct mime_info;

	init_approved();
	has_lines = 0;
	j = time(NULL);
	sprintf(id, "%08lx", j);
	hd = rd_para(zconnect);
	if (!hd) return;
	p = find(HD_EMP, hd);
	if (!p) {
		newlog(ERRLOG, "Kein Empfaenger!" );
		exit( EX_DATAERR );
	}

	skip = 0;
	pst = find(HD_STAT, hd);
	for (; pst; pst=pst->other) {
		if (stricmp(pst->text, "ZNETZ") == 0) {
			/*
			 *  Ok, es war eine RFC- oder ZCONNECT-Nachricht,
			 *  (jedenfalls nicht Z3.8), und sie wurde per Z3.8-
			 *  Transport verstuemmelt.
			 */
			habs = find(HD_ABS, hd);
			skip = 1;
			if (habs)	/* Kam sie ueber einen Gate? */
				skip = strchr(habs->text, '%') != NULL;
			break;
		}
	}
	if (skip) {
		newlog(ERRLOG, "Z3.8 Cross-Routing abgewiesen %s", p->text);
		p = find(HD_LEN, hd);
		if (!p) return;
		len = atol(p->text);
		free_para(hd);
		while (len)
			len -= fread(smallbuffer, 1, SMALLBUFFER > len ?
				len : SMALLBUFFER-1, zconnect);
		return;
	}

#ifdef REAL_GATE
	pst = find(HD_X_DONT_GATE_IT, hd);
	if (pst) {
		skip = 1;
	}
	if (skip) {
		newlog(ERRLOG, "RFC->ZC->RFC Routing abgewiesen %s", p->text);
		p = find(HD_LEN, hd);
		if (!p) return;
		len = atol(p->text);
		free_para(hd);
		while (len)
			len -= fread(smallbuffer, 1, SMALLBUFFER > len ?
				len : SMALLBUFFER-1, zconnect);
		return;
	}
#endif

	/* Wir approven auch, wenn das Brett in einem anderen als
		dem ersten Brett steht - Moritz */
/* xxx sollte eigentlich eine &-Verknuepfung sein, aber nur fuer die Bretter,
   in denen es noetig ist. */
	do_approve=0;
	for_header(p, HD_EMP, hd)
		do_approve |= approved(p->text);
	lines = 0;
	tmp = tmpfile();
	if (!tmp) {
		newlog(ERRLOG, "Temporäre Datei nicht schreibbar!");
		exit( EX_CANTCREAT );
	}
	tmphd=tmpfile();
	if (!tmphd) {
		newlog(ERRLOG, "Temporäre Datei nicht schreibbar!");
		exit( EX_CANTCREAT );
	}
	setvbuf(tmp, NULL, (_IOFBF), (long)(BIGBUFFER) > (long)(MAXINT) ? (MAXINT) : (BIGBUFFER));
	/* Steht schon ein Approved-Header im Text? */
	p = find(HD_UU_U_APPROVED, hd);
	if(p){
		do_approve = 0;
	}
	p = find(HD_UU_U_LINES, hd);
	if(p){
		has_lines=1;
		fprintf(tmphd, HN_UU_LINES": %s%s", p->text, eol);
		hd = del_header(HD_UU_U_LINES, hd);
	}
	hd = convheader(hd, tmphd);
	if (do_approve) {
		fprintf(tmphd, "Approved: approver@%s.%s\n", boxstat.boxname, boxstat.boxdomain);
	}
	p = find(HD_TYPE, hd);
	if (p) {
		if (strlen(p->text) == 0)
			typ = dstrdup("BIN");
		else
			typ = dstrdup(p->text);
		hd = del_header(HD_TYPE, hd);
	} else
		typ = NULL;
	p = find(HD_FIL, hd);
	if (p) {
		file = dstrdup(p->text);
/* Der Header bleibt drin, um als X-ZC-FILE: durchgereicht zu werden! */
/*		hd = del_header(HD_FIL, hd); */
	} else
		file = NULL;
	p = find(HD_LEN, hd);
	len = atol(p->text);
	hd = del_header(HD_LEN, hd);

	/* CHARSET: Headerzeile auswerten */
        p = find(HD_CHARSET, hd);
        if (p) {
                if (strncasecmp(p->text, "ISO", 3) == 0) {
                        charset = (p->text)[3] - '0';
                }
                else
                        charset = -2; /* unbekannt */
                hd = del_header(HD_CHARSET, hd);
        }
        else
                charset = 0;

	comment = 0;
	p = find(HD_KOM, hd);
	if (p) {
		comment = atol(p->text);
		hd = del_header(HD_KOM, hd);
	}

        /* Falls schon MIME-Headerzeilen vohanden sind,
         * werden sie unten wieder hergestellt. Wir retten
         * vorher die relevanten Informationen daraus.
         */

        wasmime = parse_mime_header(1, hd, &mime_info);

	p = find(HD_WAB, hd);
	if (p) {
		fputs(HN_UU_SENDER": ", tmphd);
		ulputs(p->text, tmp);
		hd= del_header(HD_WAB, hd);
	}

	hd = del_header(HD_STAT, hd);
	hd = del_header(HD_PRIO, hd);
	hd = del_header(HD_GATE, hd);

	u_f_and_all(tmphd, hd);

	free_para(hd);

        /* Der Typ der Nachricht wird von evtl. schon vorhandenen
         * MIME-Informationen beeinflusst:
         * multipart bedeutet hier UnixConnect-Spezial-Multipart
         * Solches Multipart nur, wenn Content-Type stimmt
         * Kein uuencoden, wenn Content-Transfer-Encoding schon
         * da ist
         */

        multipart = 0;
        if (wasmime) {
                /* Alte MIME-Headerzeilen da */
                /* Binaerverarbeitung verhindern,
                 * wenn Encoding schon da steht
                 */
                if (mime_info.encoding != cty_none)
                        if (typ)
                                dfree(typ);
                if (comment && mime_info.unixconnect_multipart)
                        multipart=1;
        }
        else {
                if (comment && typ)
                        multipart = 1;
        }

        /* Nachrichtentext selbst lesen */
        /* (ascii-mode) */
        eightbit=0;
	bufende = bigbuffer;
	buffree = BIGBUFFER;
	if (comment && typ)
		ascii_len = comment;
	else if (typ)
		ascii_len = 0;
	else
		ascii_len = len;
	len -= ascii_len;
	/* Was ist mit Nachrichten, deren KOM-Teil nicht mit einem Zeilenende
	   abgeschlossen ist? Da wird bislang eine falsche MIME-Nachricht
	   erzeugt, bei der das Boundary nicht erkannt werden kann, weil es
	   nicht am Zeilenanfang steht. xxx */
	while (ascii_len > 0) {
		size_t read_req;
		size_t really_read;

		read_req = SMALLBUFFER > ascii_len ? ascii_len : SMALLBUFFER-1;
		if (!read_req) break;
	/* Es tauchen nachrichten auf, die (illegale) NULL-Bytes enthalten.
	   Merkwürdigerweise werden sie von den meisten ZC-Programmen
	   geroutet, also sollten wir sie auch nicht einfach kürzen. */
		really_read=fread(smallbuffer, 1, read_req, zconnect);
		for(c=smallbuffer; c<(smallbuffer+really_read); c++) {
			if (*c=='\0') { *c=' '; }
		}
	/* Hier gingen Nachrichtenteile verloren. ccr
		if (really_read < read_req) break;
	 */
		if (0 == really_read) break;
		smallbuffer[really_read] = '\0';
		ascii_len -= really_read;
		for (c=smallbuffer; *c; c++) {
			if (buffree < 5) {
				*bufende = '\0';
#ifdef USE_ISO_IN_NEWS
				if(charset == 0 && mime_info.text_plain)
					pc2iso(bigbuffer);
#endif
				fputs(bigbuffer, tmp);
				bufende = bigbuffer;
				buffree = BIGBUFFER;
			}
#ifndef USE_ISO_IN_NEWS
			if (charset == 0 && !isascii(*c)) {
				ul = strchr(umlautstr, *c);
				if (ul) {
					cv = convertstr + 2*(ul-umlautstr);
					*bufende++ = *cv++;
					*bufende++ = *cv;
					buffree -= 2;
				} else {
					*bufende++ = ' ';
					buffree--;
				}
			} else
#endif
			if (*c != '\r') {
				*bufende++ = *c;
				eightbit |= *c;
				buffree--;
				if (*c == '\n')
					lines++;
			}
		}
	}
	/* Evtl. Binärmail noch Konvertieren */
	if (typ) {
		/* uuencode */
		if (!file) {
			sprintf(smallbuffer, "%s.%d", id, ++binno);
			file = dstrdup(smallbuffer);
		}

		if (multipart) {
			sprintf(smallbuffer, "--"SP_MULTIPART_BOUNDARY"\n"
				     HN_UU_CONTENT_TYPE":  application/octet-stream\n"
				     HN_UU_CONTENT_TRANSFER_ENCODING": X-uuencode\n\n"
				     "begin 644 %s\n", file);
		} else {
			sprintf(smallbuffer, "begin 644 %s\n", file);
		}

#ifdef UUENCODE_CHKSUM
		sum = 0;
#endif
		for (c=smallbuffer; *c; c++) {
			if (buffree < 5) {
				*bufende = '\0';
				fputs(bigbuffer, tmp);
				bufende = bigbuffer;
				buffree = BIGBUFFER;
			}
			*bufende++ = *c;
			buffree--;
			if (*c == '\n')
				lines++;
		}
		for (bytecount = 0, zp = smallbuffer+1; len > 0 || bytecount > 0; ) {
			if (bytecount >= 45 || len <= 0) {
#ifdef UUENCODE_CHKSUM
				*zp++ = ENC(sum);
				sum = 0;
#endif
				*zp++ = '\n';
				*zp = '\0';
				zp = smallbuffer;
				*zp++ = ENC(bytecount);
				bytecount = 0;
				for (c=smallbuffer; *c; c++) {
					if (buffree < 5) {
						*bufende = '\0';
						fputs(bigbuffer, tmp);
						bufende = bigbuffer;
						buffree = BIGBUFFER;
					}
					*bufende++ = *c;
					buffree--;
					if (*c == '\n')
						lines++;
				}
			}
			if (len > 0) {
				memset(zbuf, 0, sizeof zbuf);
				znr = len >= 3 ? 3 : len;
				znr = fread(zbuf, 1, znr, zconnect);
				if (znr <= 0) {
					len = 0;
					continue;
				}
				bytecount += znr;
				len -= znr;
				sp = zbuf;
#ifdef UUENCODE_CHKSUM
				sum += sp[0] + sp[1] + sp[2];
#endif
				*zp++ = ENC(*sp >> 2);
				/* MB:	Klammern korrigiert,
					Bug fuehrte zu kleinbuchstaben,
					wenn erster Teil des 2. oder 3.
					Sextets 0 war. */
				*zp++ = ENC( ((*sp << 4) & 060) | ((sp[1] >> 4) & 017) );
				*zp++ = ENC( ((sp[1] << 2) & 074) | ((sp[2] >> 6) & 03) );
				*zp++ = ENC(sp[2] & 077);
			}
		}
		if (multipart)
			sprintf(smallbuffer, "``\nend\n\n--"SP_MULTIPART_BOUNDARY"\n");
		else
			sprintf(smallbuffer, "``\nend\n\n");
		for (c=smallbuffer; *c; c++) {
			if (buffree < 5) {
				if (!tmp) tmp = tmpfile();
				*bufende = '\0';
				fputs(bigbuffer, tmp);
				bufende = bigbuffer;
				buffree = BIGBUFFER;
			}
			*bufende++ = *c;
			buffree--;
			if (*c == '\n')
				lines++;
		}
	}

	*bufende = '\0';

        eightbit = eightbit & 0x80;

        /* In smallbuffer charset-Info hinlegen */
        if (eightbit)
                sprintf(smallbuffer, "ISO-8859-%d", charset ? charset : 1);
        else
                strcpy(smallbuffer, "US-ASCII");

        /* MIME-Headerzeilen */
        if (!wasmime) {
		if (eightbit || typ || multipart)
		{
	                fputs(HN_UU_MIME_VERSION": 1.0\n", tmphd);
			if (typ) {
				if (multipart) {
					fputs(HN_UU_CONTENT_TYPE": multipart/mixed; boundary=\""SP_MULTIPART_BOUNDARY"\"\n", tmphd);
				} else {
					fprintf(tmphd, HN_UU_CONTENT_TYPE": application/octet-stream; name=\"%s\"\n", file);
				}
			} else {
					fprintf(tmphd, HN_UU_CONTENT_TYPE": text/plain; charset=%s\n", smallbuffer);
			}
		}
	}

        /* Content-Transfer-Encoding */
        if (!multipart && (!wasmime || (wasmime && mime_info.encoding == cty_none))) {
                if (typ && !multipart)
                        fputs(HN_UU_CONTENT_TRANSFER_ENCODING": X-uuencode\n", tmphd);
                else
                        if (eightbit)
                                fputs(HN_UU_CONTENT_TRANSFER_ENCODING": 8bit\n", tmphd);
        }

#ifdef USE_ISO_IN_NEWS
	if (charset == 0 && mime_info.text_plain)
		pc2iso(bigbuffer);
#endif
	fputs(bigbuffer, tmp);

	/* Headerzeilen-Ende */
	fputs("\n", tmphd);
	if (multipart) {
		fputs("--"SP_MULTIPART_BOUNDARY"\n", tmphd);
		fprintf(tmphd, HN_UU_CONTENT_TYPE": text/plain; charset=%s\n", smallbuffer);
		if (eightbit) {
			fputs(HN_UU_CONTENT_TRANSFER_ENCODING": 8bit\n", tmphd);
			lines++;
		}
		fputs("\n", tmphd);
		lines+=3;
	}
	len = ftell(tmp) + ftell(tmphd);

	*lines_line = 0;
	if (!has_lines)
		sprintf(lines_line, HN_UU_LINES": %ld\n", (long)lines);
	fprintf (news, "#! rnews %ld\n", (long)(len+strlen(lines_line)));
	fputs(lines_line, news);
	fseek(tmphd, 0, SEEK_SET);
	while (!feof(tmphd)) {
		len = fread(bigbuffer, 1, BIGBUFFER, tmphd);
		fwrite(bigbuffer, len, 1, news);
	}
	fseek(tmp, 0, SEEK_SET);
	while (!feof(tmp)) {
		len = fread(bigbuffer, 1, BIGBUFFER, tmp);
		fwrite(bigbuffer, len, 1, news);
	}
	fclose(tmp);
	fclose(tmphd);
}

