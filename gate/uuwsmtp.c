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
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */


/*
 * uuwsmtp - erzeugt einen SMTP-Batch aus einer ZCONNECT-Datei
 *
 */

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
#include <time.h>
#include <errno.h>

#include "utility.h"
#include "lib.h"
#include "boxstat.h"
#include "ministat.h"
#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "uulog.h"
#include "mime.h"
#include "zconv.h"
#include "gtools.h"
#include "sysexits2.h"


int main_is_mail = 1;	/* Ja, wir sind fuer Mail's zustaendig */
const char eol[] = "\r\n";

void convert(FILE *, FILE *);

/* Wenn es ein Point ist: die Liste der erlaubten Absender */
header_p pointuser;
char *pointsys;		/* Name des Point-Systems */

static char *bigbuffer	= NULL;
static char *smallbuffer= NULL;

#ifdef DISABLE_ISO_IN_MAIL
extern char umlautstr[], convertstr[];
#endif

const char *hd_crlf = "\r\n";

void
usage(void)
{
	fputs(
"UUwsmtp  -  RFC821/822 Batch aus ZCONNECT erzeugen\n"
"Aufrufe:\n"
"        uuwsmtp (ZCONNECT-Datei) (SMTP-Directory) [Absende-System]\n"
"          alter Standard, Eingabedatei wird geloescht,\n"
"          Ausgabedatei wird im Verzeichns erzeugt.\n"
"        uuwsmtp -f (ZCONNECT-Datei) [Absende-System]\n"
"          Ergebnis geht nach stdout, Eingabedatei wird geloescht,\n"
"        uuwsmtp -d (ZCONNECT-Datei) (SMTP-Datei) [Absende-System]\n"
"          Modus mit hoechster Sicherheit, oder zum Testen.\n"
"        uuwsmtp -p [Absendesystem]\n"
"          Echte Pipe\n"
, stderr);
	exit( EX_USAGE );
}

void
do_help(void)
{
	fputs(
"UUwsmtp  -  convert from zconnect to RFC821/822 BSMTP mail batch\n"
"usage:\n"
"        uuwsmtp ZCONNECT-file BSMTP-dir [ fqdn-zconnect-host ]\n"
"          old interface, ZCONNECT-file will be deleted\n"
"          output-file will be created in the specified directory\n"
"        uuwsmtp -f ZCONNECT-file [ fqdn-zconnect-host ]\n"
"          old interface, BSMTP output will be written to stdout\n"
"        uuwsmtp -d ZCONNECT-file BSMTP-file [ fqdn-zconnect-host ]\n"
"          secure mode, no delete, no directory search.\n"
"        uuwsmtp -p [ fqdn-zconnect-host ]\n"
"        uuwsmtp --pipe [ fqdn-zconnect-host ]\n"
"          full Pipe\n"
"        uuwsmtp --output BSMTP-file [ --remove ] ZCONNECT-file\n"
"                [ fqdn-zconnect-host ]\n"
"          gnu standard\n"
"        uuwsmtp --version\n"
"          print version and copyright\n"
"        uuwsmtp --help\n"
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

	initlog("uuwsmtp");
	pointuser = NULL;
	pointsys = NULL;
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
					do_version( "UUwsmtp" );
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

	if (pointsys)
		fprintf(fout, "HELO %s\r\n", pointsys);
	else
		fprintf(fout, "HELO %s.%s\r\n",
				boxstat.boxname, boxstat.boxdomain);
	while (!feof(fin))
		convert(fin, fout);
	fputs("QUIT\r\n", fout);

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

void
convert(FILE *zconnect, FILE *smtp)
{
	header_p hd, p, habs;
	FILE *tmp;
	char buffer[300];
	const char *mid;
	char zbuf[4], *sp, *zp, *bufende, *c, *file, *typ;
	static int binno = 0;
	long comment, len, ascii_len;
	size_t buffree;
	size_t bytecount, znr;
	int start_of_line, multipart;
	int qualify, local;
	int wasmime, charset, eightbit, ctl, err;
	mime_header_info_struct mime_info;
#ifdef ENABLE_UUENCODE_CHKSUM
	int sum;
#endif

	c = NULL;
	qualify = 0;
	ctl=0;
	err=0;
	tmp = NULL;

	hd = rd_para(zconnect);
	if (!hd) return;

	p = find(HD_LEN, hd);
	len = p ? atol(p->text) : 0;
	hd = del_header(HD_LEN, hd);
	p = find(HD_MID, hd);
	mid = p ? p->text : "-";
	habs = find(HD_WAB, hd);
	if (!habs) habs = find(HD_ABS, hd);
	if (!habs) {
		newlog(ERRLOG, "mid=<%s> no ABS header in message", mid );
		exit( EX_DATAERR );
	}
/* Nachrichten mit STAT: CTL bekommen einen leeren Envelope-Absender. */
	p = find(HD_STAT, hd);
	while(p) {
	  if (!strcasecmp(p->text, "CTL"))
	    ctl=1;
	  else if (!strcasecmp(p->text, "ERR"))
	    err=1;
	  p=p->other;
	}

	for (p = habs; p; p = p->other) {
		char *s;

		strcpy(buffer, p->text);
		strlwr(buffer);
		for (s=buffer; *s; s++)
			if (isspace(*s)) break;
		*s = '\0';
		{
			s = strstr(buffer, path_qualifier);
			if (s && !s[strlen(path_qualifier)])
				qualify = 1;
			strcpy(buffer, p->text);
			for (s=buffer; *s; s++)
				if (isspace(*s)) break;
			*s = '\0';
		}
		if (pointuser) {
			/*
			 * Technisch fuer diese Mail verantwortlich
			 * ist der Default-Pointuser, also der erste
			 * in der Liste.
			 */
			strcpy(buffer, pointuser->text);
			for (s=buffer; *s; s++)
				if (isspace(*s)) break;
			*s = '\0';
		} else
		/* STAT: CTL und STAT: ERR nicht bouncen */
		if (ctl || err)
			buffer[0] = '\0';
		if (NULL==p->other)
			fprintf(smtp, "MAIL FROM:<%s>\r\n", buffer);
	}
	for (p = find(HD_EMP, hd); p; p = p->other) {
		char pbuffer[300], *at;

		newlog(Z2ULOG, "mid=<%s>, from=%s, to=%s size=%ld", \
			mid, habs->text, p->text, len);
		strncpy(buffer, p->text, 300);
       		strcpy(pbuffer, buffer);
		local = 0;
		at = strchr(buffer, '@');
		if (at) {
			list_t l;

			*at = '\0';
			strcpy(pbuffer, at+1);
			for (at = pbuffer; *at; at++)
				if (isspace(*at))
					break;
			*at = '\0';

			if (strchr(pbuffer, '.') == NULL) {
				local = 1;
			} else for (l = local_domains; l; l=l->next) {
				char *st, *pbufferlow;

				pbufferlow=dstrdup(pbuffer);
				strlwr(pbufferlow);
				if ((st = strstr(pbufferlow, l->text)) != NULL)
					if (strcmp(st, l->text) == 0) {
						local = 1;
						dfree(pbufferlow);
						break;
					}
				dfree(pbufferlow);
			}
#ifndef ENABLE_RCPT_WITH_BANG
                        strcat(buffer, "@");
                        strcat(buffer, pbuffer);
                        strcpy(pbuffer,buffer);
#else
			strcat(pbuffer, "!");
			strcat(pbuffer, buffer);
#endif
		} else {
			local = 1;
		}
		fputs("RCPT TO:<", smtp);
		fprintf(smtp, qualify||local ? "%s" : default_path, pbuffer);
		fputs(">\r\n", smtp);
	}
	fprintf(smtp,	"DATA\r\n");

	/* Header groesstenteils konvertieren */
	hd = convheader(hd, smtp);

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
	} else
		file = NULL;
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

	hd = del_header(HD_STAT, hd);
	hd = del_header(HD_UU_U_LINES, hd);

	/* U-, F- und ZConnect-eigene unbekannte
	 * Headerzeilen richtig umwandeln:
	 * (z.B. auch MIME-Headerzeilen mit U- !)
	 */
	u_f_and_all(smtp, hd);

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

	/* ascii_len fuer evtl. Zeichensatzwandlung */
	if (comment && typ)
		ascii_len = comment;
	else if (typ)
		ascii_len = 0;
	else
		ascii_len = len;
	len -= ascii_len;
	start_of_line = 1;
	while (ascii_len > 0) {
		size_t read_req;
		size_t really_read;

		read_req = SMALLBUFFER > ascii_len ? ascii_len : SMALLBUFFER-1;
		if (!read_req) break;
	/* Es tauchen nachrichten auf, die (illegale) NULL-Bytes enthalten.
	   Merkwuerdigerweise werden sie von den meisten ZC-Programmen
	   geroutet, also sollten wir sie auch nicht einfach kuerzen. */
		really_read=fread(smallbuffer, 1, read_req, zconnect);
		for(c=smallbuffer; c < (smallbuffer+really_read); c++) {
			if (*c=='\0') { *c=' '; }
		}
	/* xxx Geht hier evtl. ein Teil der Nachricht verloren, wenn der
	   LEN:-Header nicht stimmt?? */
		if (really_read < read_req) break;
		smallbuffer[read_req] = '\0';
		ascii_len -= read_req;
		if (start_of_line && *smallbuffer == '.') {
			*bufende++ = '.';
			buffree--;
		}
		for (c=smallbuffer; *c; c++) {
			if (buffree < 5) {
				/* Pufferueberlauf, in tmp-File schreiben */
				if (!tmp) tmp = tmpfile();
				*bufende = '\0';
#ifndef DISABLE_ISO_IN_MAIL
				if (mime_info.text_plain && charset == 0)
					pc2iso(bigbuffer);
#endif
				fputs(bigbuffer, tmp);
				bufende = bigbuffer;
				buffree = BIGBUFFER;
			}
#ifdef DISABLE_ISO_IN_MAIL
			if (charset == 0 && mime_info.text_plain
			&& !isascii(*c)) {
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
				start_of_line = 0;
			} else {
#else
			{
#endif
				*bufende++ = *c;
				eightbit |= *c;
				buffree--;
				if (*c == '\n') {
					start_of_line = 1;
					if (c[1] == '.') {
						*bufende++ = '.';
						buffree--;
					}
				} else {
					start_of_line = 0;
				}
			}
		}
	}

	/* ASCII-Text ist jetzt gelesen und teils in tmp,
	 * teils in bigbuffer.
	 *
	 * Jetzt kommt der Binaerteil...
	 */
	if (typ) {
		/* uuencode */
		if (!file) {
			sprintf(smallbuffer, "%s.%d", baseid, ++binno);
			file = dstrdup(smallbuffer);
		}
		if (multipart) {
			sprintf(smallbuffer, "--"SP_MULTIPART_BOUNDARY"\r\n"
			HN_UU_CONTENT_TYPE":  application/octet-stream\r\n"
			HN_UU_CONTENT_TRANSFER_ENCODING": X-uuencode\r\n\r\n"
			"begin 644 %s\r\n", file);
		} else {
			sprintf(smallbuffer, "begin 644 %s\r\n", file);
		}
#ifdef ENABLE_UUENCODE_CHKSUM
		sum = 0;
#endif
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
		}
		for (bytecount = 0, zp = smallbuffer+1;
					len > 0 || bytecount > 0; ) {
			if (bytecount >= 45 || len <= 0) {
#ifdef ENABLE_UUENCODE_CHKSUM
				*zp++ = ENC(sum);
				sum = 0;
#endif
				*zp++ = '\r';
				*zp++ = '\n';
				*zp = '\0';
				zp = smallbuffer;
				*zp++ = ENC(bytecount);
				bytecount = 0;
				if (smallbuffer[0] == '.') {
					*bufende++ = '.';
					buffree--;
				}
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
#ifdef ENABLE_UUENCODE_CHKSUM
				sum += sp[0] + sp[1] + sp[2];
#endif
			/* MB: Klammern korrigiert -
				Bug fuehrte zu Kleinbuchstaben, wenn
				erster Teil des 2. oder 3. Sextetts null war */
				*zp++ = ENC(*sp >> 2);
				*zp++ = ENC(((*sp << 4) & 060) |
						((sp[1] >> 4) & 017));
				*zp++ = ENC(((sp[1] << 2) & 074) |
						((sp[2] >> 6) & 03));
				*zp++ = ENC(sp[2] & 077);
			}
		}
		if (multipart)
			sprintf(smallbuffer, "``\r\nend\r\n\r\n"
				"--"SP_MULTIPART_BOUNDARY"\r\n");
		else
			sprintf(smallbuffer, "``\r\nend\r\n\r\n");
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
		if(eightbit || typ ||multipart)
		{
			fputs(HN_UU_MIME_VERSION": 1.0\r\n", smtp);
			if (typ) {
				if (multipart) {
					fputs(HN_UU_CONTENT_TYPE
					": multipart/mixed; boundary=\""
					SP_MULTIPART_BOUNDARY"\"\r\n", smtp);
				} else {
					fprintf(smtp, HN_UU_CONTENT_TYPE
					": application/octet-stream; "
					"name=\"%s\"\r\n", file);
				}
			} else {
				fprintf(smtp, HN_UU_CONTENT_TYPE
					": text/plain; charset=%s\r\n",
					smallbuffer);
			}
		}
	}

	/* Content-Transfer-Encoding */
	if (!multipart
	&& (!wasmime || (wasmime && mime_info.encoding == cty_none))) {
		if (typ && !multipart)
			fputs(HN_UU_CONTENT_TRANSFER_ENCODING
				": X-uuencode\r\n", smtp);
		else
			if (eightbit)
				fputs(HN_UU_CONTENT_TRANSFER_ENCODING
					": 8bit\r\n", smtp);
	}

	/* Hier ist der Header jetzt vollstaendig. */
	fputs("\r\n", smtp);

	if (multipart) {
		fputs("--"SP_MULTIPART_BOUNDARY"\r\n",smtp);
		fprintf(smtp, HN_UU_CONTENT_TYPE
			": text/plain; charset=%s\r\n", smallbuffer);
		if (eightbit)
			fputs(HN_UU_CONTENT_TRANSFER_ENCODING
				": 8bit\r\n", smtp);
		fputs("\r\n", smtp);
	}

	if (tmp) {
		size_t readlen;

#ifndef DISABLE_ISO_IN_MAIL
		if (charset==0 && mime_info.text_plain)
			pc2iso(bigbuffer);
#endif
		fputs(bigbuffer, tmp);
		fflush(tmp);
		fseek(tmp, 0, SEEK_SET);
		while (!feof(tmp)) {
			readlen = fread(bigbuffer, 1, BIGBUFFER, tmp);
			fwrite(bigbuffer, readlen, 1, smtp);
		}
		fclose(tmp);
	} else {
#ifndef DISABLE_ISO_IN_MAIL
		if (charset == 0 && mime_info.text_plain)
			pc2iso(bigbuffer);
#endif
		fputs(bigbuffer, smtp);
	}
	if (!typ && !start_of_line) {
		/* Am Ende steht ASCII-Text, der nicht mit
		 * CR/LF aufhoert
		 */
		fputs("\r\n", smtp);
	}
	fputs(".\r\n", smtp);
	if (file)
		dfree(file);
	if (typ)
		dfree(typ);
}

