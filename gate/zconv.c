/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 1999       Andreas Barth, Pfad bei Pointsystemen
 *  Copyright (C) 1999       Moritz Both
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
 * zconv.c - Konvertiert einen ZCONNECT-Header nach RFC
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <ctype.h>
#include <time.h>

#include "utility.h"
#include "lib.h"
#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "uulog.h"
#include "alias.h"
#include "version.h"
#include "boxstat.h"
#include "mime.h"
#include "zconv.h"

char umlautstr[] = "\0x94\0x99\0x84\0x8e\0x81\0x9a\0xe1",
 convertstr[] = "oeOeaeAeueUess";

extern char eol[];

extern header_p pointuser;

char *
eda2date(const char *text )
{
	struct tm t;
	time_t dt;
	char s_or_w;
	char *answer;
	char buffer[24];
	int tz_hour;
	int tz_min;

	answer=dalloc(sizeof(char)*84);

	tz_hour = tz_min = 0;
	memset(&t, 0, sizeof t);
	sscanf(text, "%04d%02d%02d%02d%02d%02d%c%d:%d",
		&t.tm_year, &t.tm_mon, &t.tm_mday,
		&t.tm_hour, &t.tm_min, &t.tm_sec,
		&s_or_w, &tz_hour, &tz_min);

	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_isdst = -1;

/*
        Ziel: die lokale Zeit des Absenders

        Wir bilden mit mktime einen Zeitwert, als ob es unsere
        eigene lokale Zeit waere. Dies liefert uns einen Fehler
        um die Differez der Zeitzonen von Absender und Gate.
        Wir korregieren den Zeitwert mit der Zeitzone des Absenders.
        Der Restfehler ist nun unsere eigene Zeitzone zu GMT.

        Mit localtime bilden wir nun die tm-Struktur neu. Da unser
        Zeitwert aber golobal war ist das Ergebnis um unsere eigene
        Zeitzone verschoben. Das Resultat ist die Zeit des Absenders

        ueber diese Formel werden die Stunden, Wochentage, Datumsuebegaenge,
        Schaltjahre und sonstige Probleme einfach geloest.

	Dirk Meyer
*/

	dt = mktime(&t);
	dt += ( ( ((long)(tz_hour) * 60L) + (long)(tz_min)) * 60L );
	t = *localtime( &dt );
	t.tm_isdst = -1;

/* und den Wochentag berechnen... */
	dt = mktime(&t);

	strftime(answer, 60, "%a, %d %b %Y %H:%M:%S", &t);
	sprintf(buffer, " %+03d%02d", tz_hour, tz_min);
	strcat(answer, buffer);

	return answer;
}

/* Hier werden die ZConnect Brettnamen auf Gueltigkeit geprueft. */
int
valid_newsgroups( const char *data )
{
	if(NULL==data)
		return 0;
	/* Bretter fangen immer mit "/" an */
	if ( data[0] != '/' )
		return 0;
	/* wenn dies eine Brett@Box-Adresse sein koennte */
	if ( strchr( data, '@' ) != NULL )
		return 0;
	return 1;
}

/*
 *  Liefert 1, wenn die beiden Adressteile (ohne Realname) identisch
 *  sind
 */
int
adrmatch(const char *abs1, const char *abs2)
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

/* Druckt EINE newsgroup nach EINEM Brett.
 * Gibt !=0 zurueck im Ungluecksfall.
 */
int
printnewsgroup(const char *brett, FILE *f)
{
	char *s;
	static char buffer[MAXLINE];

	if ( valid_newsgroups( brett ) == 0 )
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
void
printnewsgroups(header_p p, const char *uuheader, FILE *f)
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

extern int main_is_mail;
extern char *pointsys;

header_p
convheader(header_p hd, FILE *f)
{
	header_p p, t, to, cc, bcc;
	char *s, *test, *habs, *oabs, *sender;
	char *mime_name, *absname;
	int org_from, pos, nokop;

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
			if(strcasecmp(t->text,"NOKOP")==0) {
				nokop = 1;
				break;
			}
		}
	}
#if 0
	/* Nokop-Zeile sollte entfernt werden,
	   daher wird hier nicht geschrieben. */
	if (main_is_mail && nokop)
		fprintf(f, "X-ZC-STAT: NOKOP%s", eol);
#endif
	p = find(HD_EMP, hd);
	if (p) {
		int mail, news;

		for (mail=0, news=0, t=p; t && (!news || !mail); t=t->other) {
			if ( valid_newsgroups( p->text ) == 0 )
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
			if ( valid_newsgroups( p->text ) == 0 )
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
			if ( valid_newsgroups( p->text ) == 0 )
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
/* Wenn wir den Header nicht bearbeitet haben,
 * wird er als X-ZC-KOP: ausgegeben. */
#if 0
		else {
			/* Wenn wir die Headerzeile nicht bearbeitet haben,
			 * wird sie als X-ZC-KOP: ausgegeben.
			 * Das wollen wir aber NICHT mehr:
			 */
			 hd=del_header(HD_KOP, hd);
		}
#endif
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
#ifdef ENABLE_POINT_MESSAGE_ID
		if (pointuser) {
			extern char *pointsys;
			char *s;

			s = strchr(p->text, '@');
			if (s) *s = '\0';
			if(strchr(pointsys, '.') != 0)
				fprintf(f,HN_UU_MESSAGE_ID": <%s@%s>%s",
					p->text, pointsys, eol);
			else
				fprintf(f, HN_UU_MESSAGE_ID": <%s@%s.%s.%s>%s",
					p->text, pointsys, boxstat.boxname,
					boxstat.boxdomain, eol);
			if (s) *s = '@';
		} else
#endif /* ENABLE_POINT_MESSAGE_ID */
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
	habs = NULL;
	sender = NULL;
	p = find(HD_OAB, hd);
	if (p) {
		/* Bei Mail wird OAB zum From:
		 * bei news wird hier der Sender transportiert.
		 * (bei WAB:/ABS: passt Sender:/From:,
		 */
		habs = dstrdup(p->text);
		p = find(HD_ABS, hd);
		if (p) {
			sender = dstrdup(p->text);
		}
		hd = del_header(HD_OAB, hd);
		hd = del_header(HD_ABS, hd);
	} else {
		p = find(HD_ABS, hd);
		if (p) {
			habs = dstrdup(p->text);
			p = find(HD_WAB, hd);
			if(p)
			{
				sender = dstrdup(p->text);
			}
			hd = del_header(HD_ABS, hd);
			hd = del_header(HD_WAB, hd);
			hd = del_header(HD_OAB, hd);
		}
	}
	if (pointuser) {
		for (t = pointuser; t; t=t->other)
			if (adrmatch(habs, t->text)) {
				/*
				 * Ok, wegen Schreibweise aber durch unsere
				 * Version ersetzen
				 */
				dfree(habs);
				habs = dstrdup(t->text);
				break;
			}
		/*
		 *  Point mit falscher Absenderangabe: ersetzen durch
		 *  den ersten Point-User: aus der Liste (das ist der Default-
		 *  User)
		 */
		if (!t) {
			dfree(habs);
			habs = dstrdup(pointuser->text);
		}
	}
	if(habs)
		oabs = dstrdup(habs);
	else /* fehlerhafte Eingabedatei, kein ABS:-Header */
	{
#ifndef DISABLE_LOG_X_HEADER
		fprintf(f, "X-Gate-Error: No ABS%s", eol);
#endif
		oabs = dstrdup("");
		habs = dstrdup("");
	}
	if (!habs || !oabs) {
		out_of_memory(__FILE__);
	}
	s = strchr(habs, ' ');
	if (s) *s = '\0';
	strlwr(habs);
	test = strrchr(habs, '@');
/* ist das hier echt noch noetig? -- ccr */
	if (test && strcmp(test, "@uucp")==0) {
		*test = '\0';
		test = strrchr(habs, '%');
		if (test) *test = '@';
		dfree(oabs);
		oabs = dstrdup(habs);
	} else
		if (s) *s = ' ';
	if (!org_from) {
		pc2iso(oabs);
		mime_name=mime_address(oabs);
		foldputs(f, HN_UU_FROM, mime_name);
		dfree(mime_name);
		hd = del_header(HD_UU_U_FROM, hd);
	}
	if (sender) {
		pc2iso(sender);
		mime_name=mime_address(sender);
		fprintf(f, HN_UU_SENDER ": %s%s", mime_name, eol);
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
		for(t=p; t; t=t->other) {
			if (*(t->text))
			{
			  pc2iso(t->text);
			  mime_name=mime_address(t->text);
			} else {
			  /* habs wird hier zerstoert */
			  pc2iso(habs);
			  mime_name=mime_address(habs);
			}
			fprintf(f, HN_UU_RETURN_RECEIPT_TO ": %s%s",
				mime_name, eol);
			dfree(mime_name);
		}
		hd = del_header(HD_EB, hd);
	}
	dfree(habs);
	dfree(oabs);
	if (sender)
		dfree(sender);
	p = find(HD_ORG, hd);
	if (p) {
	        pc2iso(p->text);
		mime_name=mime_encode(p->text);
		foldputs(f, HN_UU_ORGANIZATION, mime_name);
		dfree(mime_name);
		hd = del_header(HD_ORG, hd);
	}
	p = find(HD_BET, hd);
	if (p) {
		/* Umlaute werden nicht mit ae... ausgegeben, um
		 * ein transparentes Gating zu ermoeglichen. */
		char *encoded;
		pc2iso(p->text);
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

			buffer = eda2date(p->text);
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

			buffer = eda2date(p->text);
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
	  pc2iso(p->text);
	  encoded=mime_encode(p->text);
	  foldputs(f, HN_UU_X_MAILER, encoded);
	  dfree(encoded);
	  hd = del_header(HD_MAL, hd);
	}
	p = find(HD_BEZ, hd);
	if (p) {
	  /* References enthalten keinen Whitespace
	   * und duerfen daher nicht umbrochen werden.
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
		fprintf(f, "X-Gateway: ZCONNECT %s.%s ["
			MAILER " " VERSION "], NETCALL3.8 %s",
			boxstat.boxname, boxstat.boxdomain, p->text);
	    } else {
		fprintf(f, "X-Gateway: ZCONNECT %s.%s ["
			MAILER " " VERSION "]",
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
	hd = del_header(HD_ZNETZ_TEXT, hd);
	hd = del_header(HD_MAL, hd);
	hd = del_header(HD_UU_U_TO, hd);
	hd = del_header(HD_UU_U_CC, hd);
	hd = del_header(HD_UU_U_BCC, hd);
	hd = del_header(HD_GATE, hd);
	hd = del_header(HD_PRIO, hd);

	/* Fido: F-To: nach X-Comment-To: */
	p = find(HD_F_TO, hd);
	if (p) {
		foldputs(f, HN_UU_X_COMMENT_TO, p->text);
		hd = del_header(HD_F_TO, hd);
	}

	return hd;
}

/*
 * Gibt einen String inhalt aus, der gemaess RFC822 umbrochen wird.
 * Ein '\t' im auszugebenden String erzwingt einen Umbruch.
 * Innerhalb eines "" quoted-strings ist es nicht siinnvoll.
 * *col gibt eine Spalte an, in der wir beginnen und steht nach der
 * Rueckkehr auf der derzeitigen Position.
 *
 * Gibt =kein= Zeilenende-Zeichen aus! (Im Gegensatz zu foldputs(), s.u.)
 * (C) Moritz Both
 */

void
foldputs_no_eol(FILE *f, const char *inhalt, int *col)
{
#if ENABLE_NEW_BREAK
	int i;
	int in_quoted_string, in_quoted_string2;
	char *p, *p1, *lastspace;
	char ch;

	p=inhalt;
	while (*p) {
		lastspace=NULL;
		in_quoted_string=0;
		/* move MAXCOL characters forward; if not in quoted-string,
		 * stop at a tab character and rememer position of final space
		 */
		for(p1=p; *p1 && (*col)<MAXCOL; p1++, (*col)++) {
			if (*p1 == '\"')
				in_quoted_string = !in_quoted_string;
			if (! in_quoted_string) {
				if (*p1 == '\t')
					break;
				if (*p1 == ' ')
					lastspace=p1;
			}
		}
		if ((*col) == MAXCOL && *p1 != '\t') {
			/* There was no tab char. See if there will be a tab
			   or the end of the string
			   within the next 7 characters, then we wrap there.
			 */
			in_quoted_string2 = in_quoted_string;
			for (i=0;
				i<7 && p1[i] &&
				(p1[i] != '\t' || in_quoted_string2);
				i++)
				if (p1[i] == '\"')
					in_quoted_string2 = !in_quoted_string2;
			if (i<7) {
				p1+=i;
			} else {
				/* Move to last space, if any */
				if (lastspace) {
					 p1=lastspace;
				} else {
					/* else, look for next one */
					while(*p1 && (!isspace(*p1) ||
							in_quoted_string)) {
						if (*p1 == '\"')
							in_quoted_string =
							    !in_quoted_string;
						p1++;
				}
			}
		}
		ch = *p1;
		*p1='\0';
		fputs(p, f);
		*p1=ch;
		while(*p1 && isspace(*p1))
			p1++;
		if (*p1) {
			fputs(eol, f);
			putc('\t', f);
			(*col)=10;
		}
		p = p1;
	}
#else
	const char *p;

	for (p=inhalt; *p; p++)
		if (isspace(*p) && (*col) > 60) {
			fputs(eol, f); fputs("\t", f);
			*col = 10;
		} else {
			putc(*p, f);
			(*col)++;
		}
#endif
}

/*
 * Gibt einen Header RFC822-umbrochen aus. Wenn hd != NULL, wird
 * der Name des Headers, gefolgt von einem ": ", vorangestellt.
 * ansonsten wird davon ausgegangen, dass eine neue Zeile begonnen
 * werden soll, das Zeilenende aber schon da ist, es wird also mit
 * einem '\t' begonnen. Gibt am Ende ein eol aus.
 */

void
foldputs(FILE *f, const char *hd, const char *inhalt)
{
	int col;

	if(NULL != hd)
	{
		fputs(hd, f); fputs(": ", f);
		col = strlen(hd)+2;
	} else {
		fputs("\t", f);
		col = 10;
	}
	foldputs_no_eol(f, inhalt, &col);
	fputs(eol, f);
}

/*
 * Gibt einen Header aus, der aus mehreren gleichartigen
 * ZC-Headern zusammengesetzt ist. hd wird wie in foldputs
 * verwendet.
 */
void
foldputh(FILE *f, const char *hd, header_p t)
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
 * diejenigen ausgegeben, die ein '@' enthalten, sie werden dafuer
 * durch mime_encode "gejagt".
 */
void
foldputaddrs(FILE *f, const char *hd, header_p t)
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
		if ( valid_newsgroups( p->text ) == 0 ) {
			if (col > 40) {
				fputs(eol, f); fputs("\t", f);
				col = 10;
			}
			rna=index(p->text,'(');
			if(rna) {
			  pc2iso(rna);
			}
			mime_name=mime_address(p->text);
			fprintf(f, "%s%s", mime_name, p->other ? ", " : "");
			col += strlen(mime_name) + (p->other ? 2:0);
			dfree(mime_name);
		}
	}
	fputs(eol, f);
}

void
u_f_and_all(FILE *f, header_p hd)
{
	header_p p, p1;
	char *text;

        /* U-, F- und ZConnect-eigene unbekannte
         * Headerzeilen richtig umwandeln:
         */
	for (p=hd; p; p=p->next)
		for (p1=p; p1; p1=p1->other) {
			if(strncasecmp(p1->header,"U-",2) == 0) {
				pc2iso(p1->text);
				text=mime_encode(p1->text);
				foldputs(f,((p1->header)+2),text);
				dfree(text);
				continue;
			}
			if(strncasecmp(p1->header,"F-",2) == 0) {
				fputs("X-FTN-", f);
				pc2iso(p1->text);
				text=mime_encode(p1->text);
				foldputs(f,((p1->header)+2),text);
				dfree(text);
				continue;
			}
#ifndef ENABLE_U_X_HEADER
			/* TetiSoft: X- bleibt X- (Gatebau '97) */
			if (strncasecmp(p1->header,"X-",2) != 0)
				fputs("X-ZC-", f);
#else
			fputs("X-ZC-", f);
#endif
			pc2iso(p1->text);
			text=mime_encode(p1->text);
			foldputs(f, p1->header, text);
			dfree(text);
		}
}


