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
 * zconv.c - Konvertiert einen ZCONNECT-Header nach RFC
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
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
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <ctype.h>

#include "utility.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"

#include "uulog.h"
#include "alias.h"
#include "version.h"
#include "boxstat.h"
#include "mime.h"

void foldputs(FILE *, char *, char *);
void foldputh(FILE *f, char *hd, header_p t);
void foldputaddrs(FILE *f, char *hd, header_p t);
char umlautstr[] = "\x94\x99\x84\x8e\x81\x9a\xe1",
 convertstr[] = "oeOeaeAeueUess";
extern char eol[];

extern header_p pointuser;

/*
 *  Liefert 1, wenn die beiden Adressteile (ohne Realname) identisch
 *  sind
 */
int adrmatch(char *abs1, char *abs2)
{
	while (toupper(*abs1) == toupper(*abs2)) {
		abs1++; abs2++;
		if (!*abs1 || !*abs2 || *abs1 == ' ' || *abs2 == ' ')
			break;
	}
	if (!*abs1 || *abs1 == ' ')
		return (!*abs2 || *abs2 == ' ');
	return 0;
}

void ulputs(char *text, FILE *f)
{
#ifdef MIME_ENC_HEADER
	char *encoded;
	
	encoded=mime_encode(text);
	fputs(encoded, f);
	fputs(eol, f);
	dfree(encoded);
#else
	char *s, *ul, *cv;

	for (s=text; *s; s++)
		if (isascii(*s))
			putc(*s, f);
		else if ((ul = strchr(umlautstr, *s)) != NULL) {
			cv = convertstr + 2*(ul-umlautstr);
			putc(*cv++, f);
			putc(*cv, f);
		} else
			putc(' ', f);
	fputs(eol, f);
#endif
}		

/* Druckt EINE newsgroup nach EINEM Brett.
 * Gibt !=0 zurueck im Ungluecksfall.
 */
int printnewsgroup(char *brett, FILE *f) {
	char *s;
	static char buffer[MAXLINE];

	if (strchr(brett, '@')) 
		return 1;
	s = z_alias(brett);
	if (s) {
		fputs(s, f);
		return 0;
	}
	s = z_prefix(brett);
	if (s) {
		strcpy(buffer, s);
		strcat(buffer, (brett)+prefix_len);
	} else {
		strcpy(buffer, (brett)+1);
	}
	strlwr(buffer);
	for(s=buffer; *s; s++)
		if (*s == '/')
			*s = '.';
	fputs(buffer, f);
	return 0;
}

/* Druckt in file f alle newsgroups, die als Bretter in 
 * den Headerzeilen p ... p->other usw. stehen.
 */
static void printnewsgroups(header_p p, char *uuheader, FILE *f)
{
	header_p t;

	if (uuheader)
		fputs(uuheader, f);
	fputs(": ", f);
	for (t=p; t; t=t->other) {
		if (printnewsgroup(t->text, f) == 0
			&& t->other)
			putc(',', f);
	}
	fputs(eol, f);
}

header_p convheader(header_p hd, FILE *f)
{
	header_p p, t, to, cc, bcc;
	char *s, *test, *abs, *oabs, *sender;
	char *mime_name, *absname;
	int org_from, pos, nokop;
	extern int main_is_mail;

	minireadstat();
	org_from = 0;
	nokop = 0;
	to = find(HD_UU_U_TO, hd);
	cc = find(HD_UU_U_CC, hd);
	bcc = find(HD_UU_U_BCC, hd);
	p = find(HD_UU_U_RECEIVED, hd);
	if (p) {
		while (p) {
			foldputs(f, p->header+2, p->text);
			p = p->other;
		}
		hd = del_header(HD_UU_U_RECEIVED, hd);
	}
	p = find(HD_STAT, hd);
	if(p) {
		for(t=p; t; t=t->other) {
			if(strcasecmp(t->text,"NOKOP")==0)
				nokop = 1;
		}
	}
	p = find(HD_EMP, hd);
	if (p) {
		int mail, news;
		
		for (mail=0, news=0, t=p; t && (!news || !mail); t=t->other) {
			if (strchr(p->text, '@'))
				mail = 1;
			else
				news = 1;
		}
		if (mail && !to) {
			foldputaddrs(f, nokop? HN_UU_BCC:HN_UU_TO, p);
		}
		if (news) {
			/*
			 * Newsgroups: in News als Hauptempfaenger
			 */
		   	printnewsgroups(p, HN_UU_NEWSGROUPS, f);
		}
		hd = del_header(HD_EMP, hd);
	}
	p = find(HD_OEM, hd);
	if (p) {
		int mail;
		
		for (mail=0, t=p; t && !mail; t=t->other)
			if (strchr(p->text, '@'))
				mail = 1;
		if (mail) {
			foldputaddrs(f, HN_UU_ORIG_TO, p);
/* Der Header wird nur geloescht, wenn wir den Inhalt auch bearbeitet haben. */
			hd = del_header(HD_OEM, hd);
		}
	}
	/* Nachrichten mit STAT: NOKOP haben eh keine KOP:-Header zu haben. */
	p = find(HD_KOP, hd);
	if (p) {
	  if(nokop)
	    hd = del_header(HD_KOP, hd);
	  else {
		int mail, news;
		
		for (mail=0, news = 0, t=p; t && !mail; t=t->other)
			if (strchr(p->text, '@'))
				mail = 1;
			else
				news = 1;
		if (mail && !cc) {
			foldputaddrs(f, HN_UU_CC, p);
			hd = del_header(HD_KOP, hd);
		}
		if (main_is_mail && news) {
			/*
			 *  Newsgroups: als Kopienempfaenger in der Mail
			 */
			printnewsgroups(p, HN_UU_NEWSGROUPS, f);
			hd = del_header(HD_KOP, hd);
		}
/* Wenn wir den Header nicht bearbeitet haben, wird er als X-ZC-KOP: ausgegeben. */
	  }
	}
	if (to) {
		foldputh(f, HN_UU_TO, to);
	}
	if (cc) {
		foldputh(f, HN_UU_CC, cc);
	}
	if (bcc) {
		foldputh(f, HN_UU_BCC, bcc);
	}
	p = find(HD_MID, hd);
	if (p) {
#ifdef CARE_FOR_POINT_MIDS
		if (pointuser) {
			extern char *pointsys;
			char *s;

			s = strchr(p->text, '@');
			if (s) *s = '\0';
			if(strchr(pointsys, '.') != 0)
				fprintf(f,HN_UU_MESSAGE_ID": <%s@%s>%s", p->text, pointsys, eol);
			else
				fprintf(f, HN_UU_MESSAGE_ID": <%s@%s.%s.%s>%s", p->text, pointsys, boxstat.boxname, boxstat.boxdomain, eol);
			if (s) *s = '@';
		} else 
#endif /* CARE_FOR_POINT_MIDS */
			fprintf(f, HN_UU_MESSAGE_ID": <%s>%s", p->text, eol);
		hd = del_header(HD_MID, hd);
	}
	p = find(HD_ERSETZT, hd);
	if (p) {
		fprintf(f, HN_UU_SUPERSEDES": <%s>%s", p->text, eol);
		hd = del_header(HD_ERSETZT, hd);
	}

	p = find(HD_CONTROL, hd);
	if (p) {
		/* CONTROL: nur auswerten, wenn es kein U-Control
		 * gibt und es nur einmal da ist
		 */
		t = find(HD_UU_U_CONTROL, hd);
		if (t || p->other)
			; 
		else {
			/* CONTROL ist nur einmal da */
			/* Parameter ansteuern. */
			for (s = p->text; *s && !isspace(*s); s++);
			for (           ; *s &&  isspace(*s); s++);

			if (strncasecmp(p->text,"CANCEL",sizeof("CANCEL")-1) == 0) {
				fprintf(f, HN_UU_CONTROL": cancel <%s>%s", s, eol);
			}
			else if (strncasecmp(p->text, "ADD", sizeof("ADD")-1) == 0) {
				fputs(HN_UU_CONTROL": newgroup ", f);
				printnewsgroup(s, f);
				fputs(eol, f);
			}
			else if (strncasecmp(p->text, "DEL", sizeof("DEL")-1) == 0) {
				fputs(HN_UU_CONTROL": rmgroup ", f);
				printnewsgroup(s, f);
				fputs(eol, f);
			}
			else {
				fprintf(f, "X-ZC-"HN_CONTROL": %s%s", p->text, eol);
			}
		}
		hd = del_header(HD_CONTROL, hd);
	}
	abs = NULL;
	sender = NULL;
	p = find(HD_OAB, hd);
	if (p) {
		abs = dstrdup(p->text);
		p = find(HD_ABS, hd);
		if (p) {
			sender = dstrdup(p->text);
		}
		hd = del_header(HD_OAB, hd);
		hd = del_header(HD_ABS, hd);
	} else {
		p = find(HD_ABS, hd);
		if (p) {
			abs = dstrdup(p->text);
			p = find(HD_WAB, hd);
			if(p)
			{
				sender = dstrdup(p->text);
			}
			hd = del_header(HD_ABS, hd);
			hd = del_header(HD_WAB, hd);
		}
	}
	if (pointuser) {
		header_p p;

		for (p = pointuser; p; p=p->other)
			if (adrmatch(abs, p->text)) {
				/* 
				 * Ok, wegen Schreibweise aber durch unsere
				 * Version ersetzen
				 */
				dfree(abs);
				abs = dstrdup(p->text);
				break;
			}
		/*
		 *  Point mit falscher Absenderangabe: ersetzen durch
		 *  den ersten Point-User: aus der Liste (das ist der Default-
		 *  User)
		 */
		if (!p) {
			dfree(abs);
			abs = dstrdup(pointuser->text);
		}
	}
	if(abs)
		oabs = dstrdup(abs);
	else /* fehlerhafte Eingabedatei, kein ABS:-Header */
	{
#ifdef LOG_ERRORS_IN_HEADERS
		fprintf(f, "X-Gate-Error: No ABS%s", eol);
#endif
		oabs = dstrdup("");
		abs = dstrdup("");
	}
	if (!abs || !oabs) {
		out_of_mem(__FILE__,__LINE__);
	}
	s = strchr(abs, ' ');
	if (s) *s = '\0';
	strlwr(abs);
	test = strrchr(abs, '@');
/* ist das hier echt noch nötig? -- ccr */
	if (test && strncmp(test, "@uucp", 5)==0) {
		*test = '\0';
		test = strrchr(abs, '%');
		if (test) *test = '@';
		dfree(oabs);
		oabs = dstrdup(abs);
	} else
		if (s) *s = ' ';
	if (!org_from) {
		pc2iso(oabs, strlen(oabs));
		mime_name=mime_address(oabs);
		foldputs(f, HN_UU_FROM, mime_name);
		dfree(mime_name);
		hd = del_header(HD_UU_U_FROM, hd);
	}
	if (sender) {
		pc2iso(sender, strlen(sender));
		mime_name=mime_address(sender);
		fprintf(f, "Sender: %s%s", mime_name, eol);
		dfree(mime_name);
	}

	if(!main_is_mail) {
	  p = find(HD_ROT, hd);
	  if (p) {
	    if (pointuser) {
	      absname = dstrdup(oabs);
	      s = strchr(absname, '@');
	      if (s) {
		*s++ = '\0';
		fprintf(f,HN_UU_PATH": %s!%s%s", s, absname, eol);
	      } else {
		fprintf(f,HN_UU_PATH": %s%s", absname, eol);
	      }
	      dfree(absname);
	    } else {
              extern char *pointsys;
              if ( !(strlen(p->text)) && pointsys ) {
                 fprintf(f, HN_UU_PATH": %s!not-for-mail%s", pointsys, eol);
              } else {
                 s = strchr(p->text, '!');
                 if(s)
                   fprintf(f, HN_UU_PATH": %s%s", p->text, eol);
                 else
                   fprintf(f, HN_UU_PATH": %s!not-for-mail%s", p->text, eol);
              }
	    }
	    hd=del_header(HD_ROT, hd);
	  } else {
	    absname = dstrdup(oabs);
	    s = strchr(absname,'@');
	    if (s)
	      {
		*s++ = '\0';
		fprintf(f,HN_UU_PATH": %s!%s%s", s, absname, eol);
	      }
	    else
	      {
		fprintf(f,HN_UU_PATH": %s%s", absname, eol);
	      }
	    dfree(absname);
	  } 
	}
	else
	{
	  p = find(HD_ROT, hd);
	  if (p) {
	    hd=del_header(HD_ROT, hd);
	  } 
	}

	p = find(HD_EB, hd);
	if (p) {
		header_p t;
		for(t=p; t; t=t->other) {
			if (*(t->text)) 
			{
			  pc2iso(t->text,strlen(t->text));
			  mime_name=mime_address(t->text);
			} else {
			  /* abs wird hier zerstoert */
			  pc2iso(abs,strlen(abs));
			  mime_name=mime_address(abs);
			}
			fprintf(f, HN_UU_RETURN_RECEIPT_TO ": %s%s", mime_name, eol);
			dfree(mime_name);
		}
		hd = del_header(HD_EB, hd);
	}
	dfree(abs);
	dfree(oabs);
	p = find(HD_ORG, hd);
	if (p) {
	        pc2iso(p->text,strlen(p->text));
		mime_name=mime_encode(p->text);
		foldputs(f, HN_UU_ORGANIZATION, mime_name);
		dfree(mime_name);
		hd = del_header(HD_ORG, hd);
	}
	p = find(HD_BET, hd);
	if (p) {
		/* Umlaute werden nicht mit ae... ausgegeben, um
		 * ein transparentes Gating zu ermöglichen. */
		char *encoded;
		pc2iso(p->text,strlen(p->text));
		encoded = mime_encode(p->text);
		foldputs(f, HN_UU_SUBJECT, encoded);
		dfree(encoded);
		hd = del_header(HD_BET, hd);
	}
	p = find(HD_UU_U_DATE, hd);
	if (p) {
		fprintf(f, HN_UU_DATE": %s%s", p->text, eol);
		hd = del_header(HD_UU_U_DATE, hd);
		hd = del_header(HD_EDA, hd);
	} else {
		p = find(HD_EDA, hd);
		if (p) {
			char * buffer;
			int tz_hour, tz_min;

			buffer = str2eda(p->text, &tz_hour, &tz_min);
			if (buffer ==NULL)
				out_of_mem(__FILE__,__LINE__);

			fprintf(f, HN_UU_DATE": %s%s", buffer, eol);
			hd = del_header(HD_EDA, hd);
			dfree(buffer);
		}
	}
	p = find(HD_UU_U_EXPIRES, hd);
	if (p) {
		fprintf(f, HN_UU_EXPIRES": %s%s", p->text, eol);
		hd = del_header(HD_UU_U_EXPIRES, hd);
		hd = del_header(HD_LDA, hd);
	} else {
		p = find(HD_LDA, hd);
		if (p) {
			char *buffer;
			int tz_hour, tz_min;

			buffer = str2eda(p->text, &tz_hour, &tz_min);
			if(buffer ==NULL)
				out_of_mem(__FILE__,__LINE__);

			fprintf(f, HN_UU_EXPIRES": %s%s", buffer, eol);
			hd = del_header(HD_LDA, hd);
			dfree(buffer);
		}
	}
	p = find(HD_ANTWORT_AN, hd);
	if (p) {
		fputs(HN_UU_REPLY_TO": ", f);
	        for (pos = 10, t=p; t; t=t->other)
	        {
/* zur Sicherheit nach Brett schauen //// xxx */
		   mime_name=mime_address(t->text);
		   if(pos > 65)
		   {
		   	fprintf(f, "%s\t", eol);
		   	pos = 8;
		   }
		   fprintf(f, "%s%s", mime_name, t->other? ", " : "");
		   pos += strlen(mime_name) + 2;
		   dfree(mime_name);
		}
		fputs(eol,f);
		hd = del_header(HD_ANTWORT_AN, hd);
	}
	p = find(HD_DISKUSSION_IN, hd);
	if (p) {
		if (!p->other && strchr(p->text, '@')) {
	/* immer poster? //// xxx */
			fprintf(f, HN_UU_FOLLOWUP_TO": poster%s", eol);
		} else {
			printnewsgroups(p, HN_UU_FOLLOWUP_TO, f);
		}
		hd = del_header(HD_DISKUSSION_IN, hd);
	}
	p = find(HD_MAL, hd);
	if (p) {
	  char *encoded;
	  pc2iso(p->text, strlen(p->text));
	  encoded=mime_encode(p->text);
	  foldputs(f, HN_UU_X_MAILER, encoded);
	  dfree(encoded);
	  hd = del_header(HD_MAL, hd);
	}
	p = find(HD_BEZ, hd);
	if (p) {
	  /* References enthalten keinen Whitespace
	   * und dürfen daher nicht umbrochen werden.
	   */
		fputs(HN_UU_REFERENCES":", f);
		for (; p; p=p->other) {
			fprintf(f, " <%s>", p->text);
		}
		fputs(eol, f);
		hd = del_header(HD_BEZ, hd);
	}
	if (!pointuser) {
	    p = find(HD_ZNETZ_CONV, hd);
	    if (p) {
		fprintf(f, "X-Gateway: ZCONNECT %s.%s [" MAILER " " VERSION "], NETCALL3.8 %s",
			boxstat.boxname, boxstat.boxdomain, p->text);
	    } else {
		fprintf(f, "X-Gateway: ZCONNECT %s.%s [" MAILER " " VERSION "]",
			boxstat.boxname, boxstat.boxdomain);
	    }
	    p = find(HD_GATE, hd);
	    if (p) {
		fprintf(f,",%s",eol);
	    	foldputs(f, NULL, p->text);
	    } else {
		fputs(eol, f);
	    }
	}
	hd = del_header(HD_ZNETZ_CONV, hd);
	hd = del_header(HD_MAL, hd);
	hd = del_header(HD_UU_U_TO, hd);
	hd = del_header(HD_UU_U_CC, hd);
	hd = del_header(HD_UU_U_BCC, hd);

	/* Fido: F-To: nach X-Comment-To: */
	p = find(HD_F_TO, hd);
	if (p) {
		foldputs(f, HN_UU_X_COMMENT_TO, p->text);
		hd = del_header(HD_F_TO, hd);
	}

	return hd;
}

/*
 * Gibt einen Header RFC822-umbrochen aus. Wenn hd != NULL, wird
 * der Name des Headers, gefolgt von einem ": ", vorangestellt.
 * ansonsten wird davon ausgegangen, daß eine neue Zeile begonnen
 * werden soll, das Zeilenende aber schon da ist, es wird also mit einem
 * '\t' begonnen.
 */
void foldputs(FILE *f, char *hd, char *inhalt)
{
	int col;
	char *p;

	if(NULL != hd)
	{
		fputs(hd, f); fputs(": ", f);
		col = strlen(hd)+2;
	} else {
		fputs("\t", f);
		col = 10;
	}
	for (p=inhalt; *p; p++) 
		if (isspace(*p) && col > 60) {
			fputs(eol, f); fputs("\t", f);
			col = 10;
		} else {
			putc(*p, f);
			col++;
		}
	fputs(eol, f);
}

/*
 * Gibt einen Header aus, der aus mehreren gleichartigen 
 * ZC-Headern zusammengesetzt ist. hd wird wie in foldputs
 * verwendet.
 */
void foldputh(FILE *f, char *hd, header_p t)
{
	int col;
	header_p p;

	if(NULL != hd)
	{
		fputs(hd, f); fputs(": ", f);
		col = strlen(hd)+2;
	} else {
		fputs("\t", f);
		col = 10;
	}
	for (p=t; p; p=p->other)
	{
		if (col > 40) {
			fputs(eol, f); fputs("\t", f);
			col = 10;
		}
		fputs(p->text, f);
		col+= strlen(p->text);
		if (NULL!= p->other) {
			fputs(", ", f);
			col += 2;
		}
	}	
	fputs(eol, f);
}

/*
 * Gibt einen Header aus, der aus Mailadressen besteht. Es werden
 * diejenigen ausgegeben, die ein '@' enthalten, sie werden dafür
 * durch mime_encode "gejagt".
 */
void foldputaddrs(FILE *f, char *hd, header_p t)
{
	int col;
	header_p p;
	char *mime_name;
	char *rna;

	if(NULL != hd)
	{
		fputs(hd, f); fputs(": ", f);
		col = strlen(hd)+2;
	} else {
		fputs("\t", f);
		col = 10;
	}
	for (p=t; p; p=p->other) 
	{
		if(strchr(p->text, '@')) {
			if (col > 40) {
				fputs(eol, f); fputs("\t", f);
				col = 10;
			}
			rna=index(p->text,'(');
			if(rna) {
			  pc2iso(rna,strlen(rna));
			}
			mime_name=mime_address(p->text);
			fprintf(f, "%s%s", mime_name, p->other ? ", " : "");
			col += strlen(mime_name) + (p->other ? 2:0);
			dfree(mime_name);
		}
	}	
	fputs(eol, f);
}

void u_f_and_all(FILE *f, header_p hd) {
	header_p p, p1;
	char *text;

        /* U-, F- und ZConnect-eigene unbekannte
         * Headerzeilen richtig umwandeln:
         */ 
	for (p=hd; p; p=p->next)
		for (p1=p; p1; p1=p1->other) {
			if(strncasecmp(p1->header,"U-",2) == 0) {
				pc2iso(p1->text,strlen(p1->text));
				text=mime_encode(p1->text);
				foldputs(f,((p1->header)+2),text);
				dfree(text);
				continue;
			}
			if(strncasecmp(p1->header,"F-",2) == 0) {
				fputs("X-FTN-", f);
				pc2iso(p1->text,strlen(p1->text));
				text=mime_encode(p1->text);
				foldputs(f,((p1->header)+2),text);
				dfree(text);
				continue;
			}
#ifndef NO_PLUS_KEEP_X_HEADER
			fputs("X-ZC-", f);
#else
			/* TetiSoft: X- bleibt X- (Gatebau '97) */
			if (strncasecmp(p1->header,"X-",2) != 0)
				fputs("X-ZC-", f);
#endif
			pc2iso(p1->text,strlen(p1->text));
			text=mime_encode(p1->text);
			foldputs(f, p1->header, text);
			dfree(text);
		}
}


