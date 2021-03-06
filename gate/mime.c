/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1995       Moritz Both
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 2001       Detlef Würkner
 *  Copyright (C) 1999-2001  Dirk Meyer
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
 * mime.c - stellt die Routinen fuer die MIME-(de)kodierung zur Verfuegung
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h> /* nur fuer sprintf(...) */
#include <unistd.h>
#include "istring.h"
#include "idir.h"
#include <errno.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "utility.h"
#include "crc.h"
#include "header.h"
#include "mime.h"
#include "hd_def.h"
#include "uulog.h"
#include "sysexits2.h"

mime_cte_struct mime_ctes[] = {
	{ "7bit",		cte_7bit },
	{ "quoted-printable",	cte_quoted_printable },
	{ "base64",		cte_base64 },
	{ "8bit",		cte_8bit },
	{ "binary",		cte_binary },
	{ "x-uuencode",		cte_x_uuencode },
	{ (char *) NULL,	cte_unknown }
};

mime_cty_struct mime_ctys[] = {
	{ "text",		cty_text	},
	{ "multipart",		cty_multipart	},
	{ "message",		cty_message	},
	{ "application",	cty_application	},
	{ "image",		cty_image	},
	{ "audio",		cty_audio	},
	{ "video",		cty_video	},
	{ (char *) NULL,	cty_unknown	}
};

static const char *base64_charset =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int decode_base64(char *, size_t *, int*);
static int get_base64_6(char);
static int decode_quoted_printable(char *, size_t *, int *);

/*
 * zaehlt das Auftreten eines Zeichens in einem String
 */
int
count(const char *s, char c)
{
	int i=0;

	for(;*s;s++)
	{
		if(*s==c){i++;}
	}

	return i;
}

/*
 * Diese Funktion ueberprueft, ob der uebergebene String MIME-QP-kodiert
 * ist, d.h. =?...?.?.........?= als Form hat.
 */
int
is_mime(char *string)
{
	/* Wenn der String mit "=?" beginnt, mit "?=" aufhoert
	 * und zwei weitere '?' enthaelt, nehmen wir an, dass es
	 * sich um einen MIME-kodierten Text handelt.
	 * Nicht perfekt, aber wahrscheinlich gut genug.
	 */
	if(strlen(string)>8)
	{
		if(strncmp(string,"=?",2)==0)
		{
			if(strncmp((string+strlen(string)-2),"?=",2)==0)
			{
				if(count(string,'?')==4)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

/*
 * Diese Funktion ueberprueft, ob ein uebergebener String das MSB gesetzt
 * hast, also Zeichen >127 enthaelt.
 */
int
is_8_bit(const char *string)
{
	const unsigned char *s;

	for ( s=(const unsigned char *)string; *s; s++ )
	{
		if(*s & 0x80)
		{
			return 1;
		}
	}
	return 0;
}

static const char specialchar[]="()<>@,;:\"/[]?.= ";

/*
 * Diese Funktion zaehlt, wieviele Zeichen QP-kodiert werden muessen.
 */
int
count_8_bit(const char *string)
{
	int i;
	const unsigned char *s;

	i = 0;
	for ( s=(const unsigned char *)string; *s; s++ )
	{
		if((*s > 0x7E) || (strchr(specialchar,*s)!=0))
		{
			i++;
		}
	}
	return i;
}

/*
 * diese Funktion konvertiert einen ISO-String nach MIME, falls noetig.
 */
char *
mime_encode(const char *iso)
{
	char *encoded, *enc;
	const unsigned char *p;
	int len;

	/*
	 * falls noetig, wird konvertiert, ansonsten umkopiert:
	 */
	if(is_8_bit(iso))
	{
		p=(const unsigned char *)iso;
		len = strlen("=?ISO-8859-1?Q?")+
			strlen(iso)+2*count_8_bit(iso)+strlen("?=")+2;
		encoded = malloc(len * sizeof(char));
		if(!encoded)
			return NULL;
		enc = encoded; /* merken, wo der String anfaengt */
		strcpy(encoded,"=?ISO-8859-1?Q?");
		encoded += strlen("=?ISO-8859-1?Q?");

		for(p=(const unsigned char *)iso; *p; p++)
		{
			if((strchr(specialchar,*p)!=0) || (*p > 0x7E))
			{
				/*
				 * sprintf gibt die Anzahl der geschriebenen
				 * Bytes zurueck. Sollte auf jeden Fall 3 sein.
				 * unter sunos liefer sprints den string "s"
				 */
				sprintf(encoded,"=%02X",(*p));
				encoded+=3;
			}else{
				*encoded++ = *p;
			}
		}
		strcpy(encoded,"?=");
	}else{
		enc = dstrdup(iso);
	}

	return enc;
}

/*
 * Diese Funktion sollte eigentlich nicht noetig sein.
 * Viele Leute verwenden aber im ZC-Bereich Umlaute und sonstige
 * Sonderzeichen im Realnamen. Diese werden am besten verlustfrei
 * MIME-kodiert uebertragen.
 */
char *
mime_address(const char *zcon_ad)
{
	char *mime_ad, *rn, *mime_ad_start;
	char *klammer_auf;
	size_t len;

	len = strlen(zcon_ad) + 2;

	if (!is_8_bit(zcon_ad))
	{
		mime_ad = dstrdup(zcon_ad);
		return mime_ad;
	}

	if ((klammer_auf=index(zcon_ad,'('))!=NULL) {
		len = len + 2 * count_8_bit(klammer_auf+1 ) + 20;
		/* eventuell muessen alle Zeichen codiert werden
		 * (jeweils zwei Zeichen mehr) und zusaetzlich noch
		 * '=?ISO-8859-1?Q?' und '?=' eingefuegt werden.
		 */
	}

	mime_ad = dalloc(len * sizeof(char));
	mime_ad_start = mime_ad;

	for(; (*zcon_ad!='\0') && (!isspace(*zcon_ad)) && (*zcon_ad !='(') ;
			zcon_ad++)
		*mime_ad++=*zcon_ad;
	if(zcon_ad) zcon_ad=klammer_auf;
	if(zcon_ad)
	{
		int klammern, i;

		zcon_ad++;
		*mime_ad++=' ';
		*mime_ad++='(';
		klammer_auf=rindex(zcon_ad,')');
		if(klammer_auf != NULL) *klammer_auf='\0';
		rn = mime_encode(zcon_ad);
		strcpy(mime_ad, rn);
		free(rn);
		klammern = count(mime_ad_start,'(') - count(mime_ad_start,')');
		if(klammern<0)
			/* Mehr Klammern zu als auf. ARRGH!
			   Wir ueberschreiben von vorne her
			   ausreichend viele ')' durch ' '.
			   Das ist zwar nicht schoen, erzeugt aber
			   eine gueltige Adresse. */
			for(i=0; i<-klammern; i++)
				*index(mime_ad, ')')=' ';
				/* Wir wissen, dass ausreichend viele
				   davon im String sind, brauchen also
				   den Rueckgabewert von index nicht zu pruefen. */
		else
			/* Es fehlen noch Klammern zu. */
			for(i=0; i<klammern && strlen(mime_ad_start)<len; i++)
				strcat(mime_ad,")");
	}else{
		*mime_ad=0;
	}
	return mime_ad_start;
}

/* Gibt 1 zurueck, wenn decodiert wurde */
int
decode_cte(char *msg, size_t *msglenp, int *eightbit,
	mime_header_info_struct *info)
{
	switch (info->encoding) {
		case cte_none:
			break;
		case cte_7bit:
		case cte_8bit:
		case cte_binary:
			return 1;
		case cte_unknown:
			break;
		case cte_base64:
			return decode_base64(msg, msglenp, eightbit);
		case cte_x_uuencode:
			/* auch info uebergeben fuer ggf. Filename */
			return decode_x_uuencode(msg, msglenp, info);
		case cte_quoted_printable:
			return decode_quoted_printable(msg, msglenp, eightbit);
	}
	return 0;
}

/* base64 - Behandlung */

static int decode_base64(char *msg, size_t *msglenp, int *eightbit) {
	long b24l;
	char *buf, *getp, *putp;
	size_t newlen;
	long l;
	int err=0;
	int bit8 = 0;

	buf = (char *) malloc( *msglenp );
	if (!buf)
		return 0;

	putp=buf;
	getp=msg;
	newlen=0;
	l = *msglenp;

	while (l>3) {
		int v,i,equal;
		char ch=' ';

		v=0;
		b24l=0;
		equal=0;
		for (i=0; i<4 && !err; i++) {
			do {
				if (!l) {
					err=1;
					break;
				}
				ch = *getp++;
				l--;
				v = get_base64_6(ch);
			} while (v==-1 && ch != '=');
			if (l<0)
				break;
			b24l <<= 6;
			if (v>-1)
				b24l |= v;
			else
			if (ch == '=')
				equal++;
			if (equal)
				if (ch != '=')
					err=1;
		}
		if (err) {
			if (!l && !i)
				err=0;
			break;
		}

		putp[0] = (b24l & 0xff0000L) >> 16;
		putp[1] = (b24l & 0xff00) >> 8;
		putp[2] = (b24l & 0xff);
		if (!bit8)
			bit8 = ( (putp[0] | putp[1] | putp[2]) & 0x80 );
		putp+=3;
		newlen+=3;

		if (equal) {
			newlen -= equal;
			break;
		}
	}

	if (!err) {
		memcpy(msg, buf, newlen);
		*msglenp = newlen;
	}
	free(buf);
	*eightbit = bit8;
	return err ? 0 : 1;
}

static int get_base64_6(char ch) {
	char *p = strchr(base64_charset, ch);
	return p ? p-base64_charset : -1;
}

#define HEXDIGIT(x) ((x)>'9' ? (x)-'A'+10 : (x)-'0')

static int
decode_quoted_printable(char *buf, size_t *msglen, int *eightbit)
{
	char *readp;
	char *newbuf, *newbufp;
	long lc;
	int bit8 = 0;

	newbuf= (char *) malloc(*msglen);
	if (!newbuf)
		return 0;

	readp=buf;
	newbufp=newbuf;
	lc=*msglen;

	while (lc > 0) {
		switch (*readp) {
			case '=':
				readp++; lc--;
				if (lc<2) {
					free(newbuf);
					return 1;
				}
/* xxx verliert die Routine evtl. das letzte Zeichen, wenn es kodiert ist? */
				if (*readp == 13 && readp[1] == 10) {
					/* Soft line break, kein output */
				}
/* =? kann auch vorkommen xxx */
				else {
					*newbufp = (HEXDIGIT(*readp)<<4) + HEXDIGIT(readp[1]);
					if (!bit8)
						bit8 = *newbufp & 0x80;
					newbufp++;
				}
				readp+=2;
				lc-=2;
				break;
			case ' ':
			case '\t':
				/* Delete white space at end of line */
/* xxx warum? */
				{
					char *p;
					long l;
					for (p=readp+1, l=lc-1; l && (*p==' ' || *p=='\t'); l--, p++);
					if (!l) {
						/* ws am Ende der Nachricht - loeschen */
						lc=0;
					}
					else
					if (*p==13) {
						/* ws am Ende der Zeile: loeschen */
						lc = l;
						readp=p;
					}
					else {
						/* ws sonst: da lassen */
						*newbufp++ = *readp++;
						lc--;
					}
				}
				break;
			default:
				*newbufp++ = *readp++;
				lc--;
				break;
		}
	}
	*msglen = newbufp - newbuf;
	memcpy(buf, newbuf, *msglen);
	free(newbuf);
	*eightbit = bit8;
	return 1;
}

/* Diese Funktion sucht im Nachrichtenheader hd nach
 * MIME-Headerzeilen. Falls vorhanden, werden sie geparst
 * und die Informationen in der struct info abgelegt
 * (die vom Aufrufer zu Verfuegung gestellt wird).
 * Rueckgabewert: Wenn es kein MIME-Header ist, 0;
 * wenn ja, 1, und die struct info wird in jedem Fall
 * ausgefuellt.
 */

int parse_mime_header(int direction, header_p hd,
				mime_header_info_struct *info) {
	header_p p;

	info->encoding = cte_none;
	info->type     = cty_none;
	info->text_plain=1; /* default */
	info->charset  = -1;
	info->charsetname = NULL;
	info->filename = NULL;
	info->unixconnect_multipart = 0;

	p = find(direction ? HD_UU_U_MIME_VERSION : HD_UU_MIME_VERSION, hd);
	if (p) {
		/* Die Versionsnummer ignorieren wir. */
		p = find(direction ? HD_UU_U_CONTENT_TRANSFER_ENCODING :
					HD_UU_CONTENT_TRANSFER_ENCODING, hd);
		if (p) {
			/* Content-Transfer-Encoding rauskriegen */
			mime_cte_struct *mp;
			for (mp = mime_ctes; (mp->name); mp++) {
				if (stricmp(p->text, mp->name)==0) {
					info->encoding=mp->cte;
					break;
				}
			}
		}
 		p = find(direction ? HD_UU_U_CONTENT_TYPE :
					HD_UU_CONTENT_TYPE, hd);
		if (p) {
			/* Content-Type rauskriegen */
			mime_cty_struct *mp;
			char *type, *subtype, *para1name, *para1value;
			int nfields;
			size_t l;

			info->text_plain=0;
			info->charset=0;
			l = strlen(p->text);
			type= (char *) malloc(l * 4);
			if (!type)
				out_of_memory(__FILE__);
			l = strlen(p->text); /* avoid gcc -O bug */
			subtype=type+l;
			para1name=subtype+l;
			para1value=para1name+l;

			nfields = sscanf(p->text,
	"%[-A-Za-z0-9]/%[-A-Za-z0-9] ; %[-A-Za-z0-9]=%[-._A-Za-z0-9\":<]",
	type, subtype, para1name, para1value);
			for (mp = mime_ctys; (mp->name); mp++) {
				if (stricmp(type, mp->name)==0) {
					info->type = mp->cty;
					break;
				}
			}
			if (info->type == cty_text) {
				if (stricmp(subtype, "plain")==0)
					info->text_plain = 1;
				if (stricmp(para1name, "charset")==0) {
					/* Manchmal steht das
					   in Anführungszeichen... */
					if (para1value[0] == '\"') {
						unsigned char *s;
						strcpy(para1value, &para1value[1]);
						if ((s = strchr(para1value, '\"')) != NULL)
							*s = '\0';
					}
					if (stricmp(para1value, "us-ascii")==0)
						info->charset = -1;
					else {
						char ch = para1value[9];
						para1value[9]=0;
						if (stricmp(para1value,"iso-8859-")==0) {
/* Es gibt mittlerweile auch zweistellige ISO-8859-XX-Nummern, z.B. 15 */
							para1value[9] = ch;
							info->charset = atol(&para1value[9]);
						} else {
							/* Unbekannte Charsets durchreichen */
							para1value[9] = ch;
							info->charsetname = dstrdup(para1value);
						}
					}
				}
			}
			else
/* auch nicht so besonders gut. */
			if (info->type == cty_multipart) {
				if (stricmp(subtype, "mixed")==0 && stricmp(para1name, "boundary")==0
						&& stricmp(para1value, "\""SP_MULTIPART_BOUNDARY"\"")==0)
					info->unixconnect_multipart=1;
			}
			else
			if (info->type == cty_application) {
				if (stricmp(subtype, "octet-stream")==0
					&& stricmp(para1name, "name")==0) {
					char *cp;

					info->filename=dstrdup(para1value+1);
					if ((cp=strchr(info->filename, 0))>info->filename)
						*(cp-1)=0;
				}
			}
			free(type);
		}
		return 1;
	}
	else
		return 0; /* kein MIME-Version: */
}

int
decode_x_uuencode(char *msg, size_t *msglenp,
	mime_header_info_struct *info)
{
		char tmpdir[FILENAME_MAX], sikdir[FILENAME_MAX], *src, *s;
		long cnt;
		static int bin_count = 0;
		struct stat st;
		FILE *f;
		int success = 0;

		sprintf(tmpdir, "/tmp/ur.dec%d.%ld", bin_count++, (long)getpid());
		if (mkdir(tmpdir, 0700) != 0) {
			newlog( ERRLOG, "%s: %s", tmpdir, strerror(errno));
			exit( EX_CANTCREAT );
		}
#ifdef HAS_BSD_GETWD
		getwd(sikdir);
#else
		getcwd(sikdir, FILENAME_MAX-1);
#endif
		chdir(tmpdir);

		/* Find the line with "begin xxx filename"
		 * and digest it. according to uuencode(5),
		 * it must be lower-case with a space. */
		s=NULL;
		src=strstr(msg, "begin ");
		if(src) s=strchr(src,'\n');
		if(s) {
			*s='\0';
			if(*(s-1)=='\r')
				*(s-1)='\0';
		}
		if(src) src=strchr(src,' ');
		if(src) src=strchr(src+1,' ');
		if(src) src++;
		if(src && info->filename == NULL)
			info->filename = dstrdup(src);
		if(s) src=s+1; /* hinter das Ende der ersten Zeile */
		if (NULL==src) {
			newlog( ERRLOG, "no begin line in uuencoded part");
			exit( EX_DATAERR );
		}
		f = popen("uudecode", "w");
		if (!f) {
			newlog( ERRLOG,
				"popen(uudecode): %s", strerror(errno));
			exit( EX_DATAERR );
		}
		fputs("begin 600 decoded.msg\n", f);
		for (cnt=(*msglenp)-(src-msg); cnt; cnt--, src++) {
			if (*src != '\r') {
				putc(*src, f);
			}
		}
		pclose(f);


		f = fopen("decoded.msg", "r");
		if (!f) {
			newlog( ERRLOG,
				"uudecode failed (can't open file!): %s",
				strerror(errno));
			exit( EX_DATAERR );
		}
		fstat(fileno(f), &st);

		fread(msg, st.st_size, 1, f);
		*msglenp = st.st_size;
		fclose(f);

		unlink("decoded.msg");
		success++;

		chdir(sikdir);
		rmdir(tmpdir);
		return success;
}

/* Suche nach dem Teil, der gemaess RFC-2047 kodiert ist.
 * Fast eher eine Aufgabe fuer lex. :-) */
typedef struct encpart {
  const char* start;
  size_t len;
  char* charset;
  char encoding;
  char* text;
} encpart;

static encpart* find_encoded_part (const char *where) {
  const char *start, *enc, *text, *end;
  encpart *ans=malloc(sizeof(struct encpart));

  start=where;
  while(start && *start && count(start,'?')>=4) {
    /* Suche nach dem naechsten "=?". */
    for(start=index(start,'=');
	start && *start && *(start+1)!='?';
	start=index(start+1,'=')) ;
    if(start && *start) {
      enc=index(start+2,'?');
      if(!enc) return NULL;
      text=index(enc+1,'?');
      if(!text) return NULL;
      end=index(text+1,'?');
      if(!end) return NULL;
      if(*(end+1)=='=') {
	ans->start=start;
	ans->len=end-start+2;
	ans->charset=malloc((size_t)(enc-start-1));
	strncpy(ans->charset,start+2,(size_t)(enc-start-2));
	ans->charset[enc-start-2]='\0';
	ans->encoding=*(enc+1);
	ans->text=malloc((size_t)(end-text));
	strncpy(ans->text,text+1,(size_t)(end-text-1));
	ans->text[end-text-1]='\0';
	return(ans);
      }
      else
	start++;
    }
  }
  dfree(ans);
  return(NULL);
}

/* dekodiert einen nach RFC2047 kodierten String.
 * Die duerfen nicht geschachtelt auftreten, also geht das so. */

char *decode_mime_string(const char *buf) {
  struct encpart *parts;
  char *ans, *s;
  const char *rest;
  int eightbit;
  size_t length;

  parts=find_encoded_part(buf);
  if(!parts)
    return(dstrdup(buf));

  ans=malloc(strlen(buf)+1); /* wird nur kleiner. */
  if(!ans)
    out_of_memory(__FILE__);

  rest = buf;
  ans[0]='\0';
  while(parts) {
    length=strlen(parts->text);
    strncat(ans, rest, (size_t)(parts->start - rest) );
    rest=parts->start+parts->len;
    /* Es werden nur US-ASCII und ISO-8859-1 dekodiert. */
    if(strcasecmp(parts->charset,"us-ascii")==0
       || strcasecmp(parts->charset,"iso-8859-1")==0) {
      switch (parts->encoding) {
	/* es werden nur 'Q' und 'B' dekodiert. Andere sind
	 * eh noch nicht spezifiziert. */
      case 'Q':
      case 'q':
	/* '_' -> ' ' */
	for(s=parts->text; *s; s++) { if (*s=='_') *s=' '; }
	decode_quoted_printable(parts->text, &length, &eightbit);
	strncat(ans, parts->text, length);
	break;
      case 'B':
      case 'b':
	decode_base64(parts->text, &length, &eightbit);
	strncat(ans, parts->text, length);
	break;
      default:
	strncat(ans, parts->start, parts->len);
	break;
      }
    } else
      strncat(ans, parts->start, parts->len);
    dfree(parts->charset);
    dfree(parts->text);
    dfree(parts);
    parts = find_encoded_part(rest);
  }
  strcat(ans,rest);
  return(ans);
}

