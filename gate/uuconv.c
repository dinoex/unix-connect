/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 * uuconv.c - Funktionen fuer Konvwertierung von RFC nach
 * ZConnect, die fuer Mail und News (fast) gleich sind
 *
*/


#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <ctype.h>
#include <time.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "utility.h"
#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "boxstat.h"
#include "version.h"
#include "uulog.h"
#include "alias.h"
#include "mime.h"
#include "uuconv.h"
#ifdef APC_A2B
#  include "apc_a2b.h"
#endif
#include "lib.h"
#include "datelib.h"

/* FQDN des Hosts, fuer den hier ZCONNECT erzeugt wird */
extern char *fqdn;

char wab_name[MAXLINE] = "", wab_host[MAXLINE] = "", wab_domain[MAXLINE] = "";
static char sp_name[MAXLINE], sp_host[MAXLINE];
static char sp_domain[MAXLINE], rna[MAXLINE];

char *
date2eda( const char *str, FILE *fout )
{
	int tz;
	char *datum;
	struct tm *lt;
	time_t dt;

	datum = strdup(str);
	dt = parsedate(datum, &tz);
	free(datum);
	if (dt == -1) {
		tz = 0;
		dt = time(NULL);
		if ( fout != NULL )
			fprintf( fout,
				"X-Date-Error: parse error in Date: %s\r\n",
				str );
	}
	lt = localtime(&dt);
	datum = dalloc( 20 + 20 );
	sprintf(datum, "%04d%02d%02d%02d%02d%02dW%+d:%02d",
		lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
		lt->tm_hour, lt->tm_min, lt->tm_sec,
		tz/60, abs(tz%60));
	return datum;
}

/* Hier werden die nach ZConnect gewandelten Brettnamen
   auf Gueltigkeit geprueft. */
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

/* Druckt alle angegebenen newsgroups (mit Komma unterteilt)
 * in eigene ZConnect-Headerzeilen.
 * Wenn zc_header NULL ist, druckt es nur eine Zeile aus
 * der ersten angegebenen newsgroup und fertig.
 */
void
printbretter(const char *newsgroups, const char *zc_header, FILE *f)
{
	static char adr[MAXLINE];
	char *bak, *st, *q, *s;
	size_t len;

	/* bearbeite jede einzelne Newsgrous einzeln */
	bak = dstrdup( newsgroups );
	st = strtok( bak, " , \t" );
	while ( st != NULL ) {
		do {
			/* vorher ".ctl" im Brettnamen abscheiden */
			len = strlen( st ) -4;
			if ( len > 0 ) {
				if ( strncmp( st + len, ".ctl", 4 ) == 0 )
					st[ len ] = '\0';
			};
			s = u_alias(st);
			if (s) {
				if (zc_header)
					fprintf(f, "%s: ", zc_header);
				fprintf(f, "%s\r\n", s);
				break;
			};
			s = u_prefix(st);
			if (s) {
				strcpy(adr, s);
				s = adr + strlen(adr);
				st += prefix_len;
			} else {
				prefix_len = 0;
				adr[0]='/';
				s=adr+1;
			};
			/* Konvertiere diesen Gruppennamen */
			for (q=st; *q ; q++ ) {
				if (*q == '.')
					*(s++) = '/';
				else
					*(s++) = toupper(*q);
			};
			*s = '\0';
			s = adr;
			if ( valid_newsgroups( adr ) != 0 )
			{
				if (zc_header)
					fprintf(f, "%s: ", zc_header);
				fprintf(f, "%s\r\n", s);
				break;
			};
#ifndef DISABLE_LOG_X_HEADER
			if (zc_header) {
				fprintf(f, "X-Bogus-Newsgroups-%s: ",
					zc_header);
				fprintf(f, "%s\r\n", st - prefix_len );
			}
#endif
			break;
		} while ( 0 );
		st = strtok( NULL, " , \t" );
	};
	dfree( bak );

}

/*
 *   Findet das letzte '@' in einem full qualified domain name und liefert
 *     NULL -> kein @ enthalten
 *     != 0 -> Zeiger auf das '@'
 *   Hinter dem FQDN darf noch (Real Name) stehen, der seinerseits
 *   auch ein '@' enthalten kann. Dies wird nicht 'gefunden'.
 */
char *
fqdn_at(char *s)
{
	char *erg;

	for (erg=NULL; *s && !isspace(*s); s++)
		if (*s == '@')
			erg = s;
	return erg;
}

/*
 *   Findet das Komma, das diese Adresse von der naechsten Adresse
 *   trennt (z.B. in "To: x@y, a@b, c@d").
 *   Minimal (aber anscheinend hinreichend) wird RFC822 Syntax geparsed:
 *   Kommata in ( ) und in " " werden ignoriert.
 *
 *   Liefert:
 *     NULL  -> kein Komma, dies ist die letzte Adresse
 *     != 0  -> Zeiger auf das Komma
 */
char *
next_komma(char *s)
{
	int level;

	while ( *s && *s != ',' ) {
		if (*s == '"') {
			s++;
			while(*s && *s != '"')
				s++;
			if (!*s) break;
		}
		if (*s == '(') {
			s++;
			level = 1;
			while ( *s && level ) {
				if (*s == '(')
					level++;
				else if (*s == ')')
					level--;
				s++;
			}
		} else {
			s++;
		}
	}

	if (*s)
		return s;
	else
		return NULL;
}


void
splitaddr(char *rfc_addr, char *name, char *host, char *domain, char *lrna)
{
	static char adr[MAXLINE];
	char *q, *q1, *s;

	/* "Foo <blah@ee.fds>" und
	 * "fds@fds.fds (Blah)"
	 * vereinheitlichen
	 */

	*name = '\0';
	*host = '\0';
	*domain = '\0';
	*lrna = '\0';
	q = strchr(rfc_addr, '<');
	if (q) {
		strcpy(adr, q+1);
		q = strchr(adr, '>');
		if (q) *q = '\0';
		strcpy(lrna, rfc_addr);
		q = strchr(lrna, '<');
		while (isspace(*(--q) ))
			;
		q++;
		*q = '\0';
	} else {
		strcpy(adr, rfc_addr);
		/*
		 * Das hier ist noch nicht ganz korrekt. Laut RFC822
		 * ist
		 *   Muhammed (I am the greatest!) .Ali @ somewhere.edu (Muhammed Ali)
		 * eine korrekte Adresse, die auf das Postfach
		 *   Muhammed.Ali@somewhere.edu
		 * zeigt. Es ist aber schon besser als die Loesung mit isspace,
		 * da Klammern, die nicht vom Adressteil getrennt
		 * sind, erkannt werden. Solche Adressen sind zwar
		 * strenggenommen falsch, treten aber auf.
		 */
		q=adr+strcspn(adr," \t(");
		/* for (q=adr; *q && !isspace(*q); q++)
			; */
		if(*q)
		{
			s = strchr(q, '(');
			if (s) s++;
		}
		else
			s=NULL;
		*q = '\0';
		if (s) {
			strcpy(lrna, s);
			s = strrchr(lrna, ')');
			if (s) *s = '\0';
		}
	}

	/*
	 *   : Notation in ! wandeln
	 */

	for (q=adr; *q; q++)
		if (*q == ':') *q = '!';
	q = fqdn_at(adr);
	if (q) {
		int dots, pre_dots;

		*q = '\0'; pre_dots = dots = 0;
		for (s=adr; *s; s++) {
			if (*s == '.') {
				dots++;
			} else if (*s == '!') {
				pre_dots = dots;
				dots = 0;
			}
		}
		if (pre_dots && !dots)
			q = NULL;
		else
			*q = '@';
	}
	s = strchr(adr, '!');
	if (s && !q) {
		/* Reine Bang-Adresse */
		for (q=q1=NULL,s=adr; *s; s++)
			if (*s == '!') {
				if (q)
					q1 = q+1;
				q = s;
			} else if (isspace(*s))
				break;
		*s = '\0';
		if (q) *q = '\0';
		if (!q1) q1 = adr;
		if (q) {
			strcpy(name, q+1);
			strcpy(host, q1);
			q = strchr(host, '.');
			if (q) {
				*q = '\0';
				strcpy(domain, q+1);
			} else {
				domain[0] = '\0';
			}
		} else {
			host[0] = '\0';
			domain[0] = '\0';
			strcpy(name, adr);
		}
	} else if (q) {
		/* Internet-Adresse */
		*q = '\0';
		strcpy(name, adr);
		strcpy(host, q+1);
		q = strchr(host, '.');
		if (q) {
			*q = '\0';
			strcpy(domain, q+1);
		} else {
			domain[0] = '\0';
		}
	}
	 else if (!s && !q) {
	 /* Die Adresse enthaelt nur den Usernamen.
	  * Das ist zwar vollkommen illegal, mehrdeutig
	  * und unsinnig, aber das Beste, was wir tun
	  * koennen, ist wohl, den Namen einfach durchzureichen.
	  */
	  	host[0] = '\0';
	  	domain[0] = '\0';
	  	strcpy(name, adr);
	  }
}

/*
 *	max == 0 beliebig viele Adressen
 *	max > 0  maximal max Adressen. (es werden die ersten max ausgegeben)
 *
 *   return:
 *		die Anzahl der ausgegebenen Adressen (alle ungueltigen werden
 *		nicht erzeugt)
 */
int
convaddr(const char *zconnect_header, const char *rfc_addr, int max, FILE *f)
{
	char *komma, *start, *line;
	static char zaddr[MAXLINE];
	int counter;
	char *q, *rna_decoded=NULL;

	counter = 0;
	line = dstrdup(rfc_addr);
	/* leere Eingabe -> leere Ausgabe */
	sp_name[0] = sp_host[0] = sp_domain[0] = rna[0] = '\0';
	for (start = line; start && *start ; ) {
		while (isspace(*start))
			start++;
		komma = next_komma(start);
		if (komma) *komma = '\0';
		splitaddr(start, sp_name, sp_host, sp_domain, rna);
		if (sp_name[0] && sp_host[0]) {

			/* Haben wir ein oder mehrere '@' im name-Part?
			 * Das wuerde ZConnect-Software verwirren (die arme).
			 * Also nach % wandeln. Das wird in den meisten Teilen
			 * der Welt verstanden.
			 */
			while( (q=strchr(sp_name, '@')) != NULL )
				*q = '%';

			/*
			 * smail and inn akzeptieren Adressen, die auf einen
			 * '.' enden. ZC-Software tut das nicht. smail beachtet
			 * den Punkt nicht, also koennen wir ihn auch
			 * wegwerfen.
			 */
			if(sp_domain[0])
				if(sp_domain[strlen(sp_domain)-1]=='.')
					sp_domain[strlen(sp_domain)-1]='\0';
			q=index(sp_name, '@');
				if(q) { *q='\0'; }
			q=index(sp_host, '@');
				if(q) { *q='\0'; }
			q=index(sp_domain, '@');
				if(q) { *q='\0'; }
			sprintf(zaddr, "%s@%s.%s", sp_name, sp_host, sp_domain);
			if (!sp_domain[0]) strcat(zaddr, "UUCP");
			counter++;
			if(rna[0] == '"')
			{
				for(q=rna+1; *q && *q!='"'; q++)
				{
					*(q-1)=*q;
				}
				*(q-1)='\0';
			}
			if(rna[0]!='\0') {
			  rna_decoded=decode_mime_string(rna);
			  iso2pc(rna_decoded);
			}
		/* Nach RFC822 darf der Lokalteil einer Adresse mit '"'
		   geklammert sein. qmail macht das auch generell so,
		   smail verwirft diese Klammern mindestens dann nicht,
		   wenn der Lokalteil auf '.' endet. Auf ZC-Seite ist
		   `"user."@host.do.main' aber etwas anderes als
		   `user.@host.do.main'. Sigh. Also verwerfen wir die
		   Hochkommata, wenn im Realnamen kein Leerzeichen auftritt. */
			if( '"' == sp_name[0] && NULL == index(sp_name, ' ') )
			{
				for(q=sp_name+1; *q && '"' != *q; q++)
					*(q-1)=*q;
				*(q-1)='\0';
			}
			if( count(zaddr, '@') == 1 )
			{
				if (rna_decoded && rna_decoded[0])
				{
					fprintf(f, "%s: %s (%s)\r\n", zconnect_header, zaddr, rna_decoded);
				}else{
					fprintf(f, "%s: %s\r\n", zconnect_header, zaddr);
				}
			} else {
				/* Na prima. Eine illegale Adresse, die keinen
				   oder mehrere @-Zeichen enthaelt.
				   Wie ist die eigentlich bis hierhin vorgedrungen?
				   Wir koennen aber schlecht keinen entsprechenden
				   Header ausgeben...
				   Den rna lassen wir unter den Tisch fallen,
				   im Realnamenfeld darf keine weitere Klammer
				   auftauchen.
				 */
				fprintf(f, "%s: nobody@nowhere.universe (%s)\r\n",
				  zconnect_header, zaddr);
			}
			dfree(rna_decoded);
			rna_decoded=NULL;
		}
		 else if(sp_name[0]) {
		 /* Nur Username ohne @... . Absolut unbrauchbar,
		  * aber wir leiten hier ja nur weiter...
		  * Die Adresse ist eh buggy, also gar kein Aufwand
		  * mit '"' und RFC2047...
		  */
			if(rna[0])
				fprintf(f, "%s: %s (%s)\r\n", zconnect_header, sp_name, rna);
			else
				fprintf(f, "%s: %s\r\n", zconnect_header, sp_name);
			counter++;
		}
		if (!komma) break;
		if (max > 0 && counter >= max) break;
		for (start = komma+1; *start && isspace(*start); start++)
			;
	}
	dfree(line);

	return counter;
}

char *
printpath(char *reverse_path)
{
	char *p, *a, *rot;

	if (!reverse_path) {
		rot = dstrdup("");
		return(rot);
	}
	p = dstrdup(reverse_path);
	/* Der Routweg ist ebensolang wie die Adresse vorher,
	   schliesslich entsteht er daraus durch Umsortieren.
	 */
	rot = dalloc(strlen(reverse_path)+1);
	rot[0] = '\0';
	a = fqdn_at(p);
	if (a) {
		*a = '\0';
		sprintf(rot, "%s!", a+1); /* rot ist gross genug dafuer. */
	}
	a = strrchr(p, '!');
	if (a) *a = '\0';
	strcat(rot, p);
	dfree(p);
	return(rot);
}

extern int main_is_mail;

header_p
convheader(header_p hd, FILE *f, char *from)
{
	int has_wab=0;
	header_p p, t;
	char *s, *st, *e, *to;
#ifndef DISABLE_NOUSER_IN_ROT
	char *sender;
#endif

	s = st = e = to = NULL;
#ifndef DISABLE_NOUSER_IN_ROT
	sender = NULL;
#endif
	p = find(HD_X_GATEWAY, hd);
	if (p) {
		s = strstr(p->text, "NETCALL3.8");
		if (s) {
			while (s && *s && !isspace(*s))
				s++;
			while (s && *s && isspace(*s))
				s++;
			e = strchr(s, ',');
			if (e) *e = '\0';
			fprintf(f, HN_ZNETZ_CONV": %s\r\n", s);
		}
	}
	p = find(HD_UU_FOLLOWUP_TO, hd);
	if (p) {
		for (st = p->text; isspace(*st); st++)
			;
		if (stricmp(st, "poster") == 0) {
			p = find(HD_UU_SENDER, hd);
			if (!p) p = find(HD_UU_REPLY_TO, hd);
			if (!p) p = find(HD_UU_FROM, hd);
			if (p) convaddr(HN_DISKUSSION_IN, p->text, 0, f);
		} else
			printbretter(p->text, HN_DISKUSSION_IN, f);
		hd = del_header(HD_UU_FOLLOWUP_TO, hd);
	}
	p = find(HD_UU_NEWSGROUPS, hd);
	if (p) {
		printbretter(p->text, main_is_mail ? HN_KOP : HN_EMP, f);
		hd = del_header(HD_UU_NEWSGROUPS, hd);
	}
	p = find(HD_UU_MESSAGE_ID, hd);
	if (p) {
		fprintf(f, HN_MID": ");
		s=strchr(p->text, '<');
		if(s)
		{
			for (s++; *s && *s != '>'; s++)
				putc(*s, f);
		}else{
			fputs(p->text,f);
		}
		/* will niemals News-Message-IDs veraendern. Auch
		 * nicht, wenn sie keinen @ enthalten.
		 */
		s=strchr(p->text,'@');
		if((s==NULL) && main_is_mail )
		{
			fputs("@unknown.nil",f);
		}
		fputs("\r\n", f);
		hd = del_header(HD_UU_MESSAGE_ID, hd);
	} else {
		/* Hmm... eine Nachricht ohne Message-ID.
		 * Das ist fuer Mail in RFC erlaubt, aber nicht
		 * in ZConnect. Bouncen ist eine sehr schlechte
		 * Loesung, eine neue MID erfinden ist eine
		 * ziemlich schlechte Loesung. Ziemlich schlecht
		 * ist besser als sehr schlecht: */
		if(main_is_mail) {
			fprintf(f,HN_MID
				": gateway-generated.%d.%d@unknown.nil\r\n",
				rand(), getpid());
		}
	}
	p = find(HD_UU_FROM, hd);
	if (p) {
		if (convaddr(HN_ABS, p->text, 1, f) == 0) {
			/*
			 *  Illegale Nachricht - die sollte uns gar nicht
			 *  erst erreicht haben, aber weder C-News noch INN
			 *  testen sorgfaeltig.
			 *
			 *  Jetzt stehen wir ohne Absender da!
			 *
			 *  Heiko Schlichting schlaegt vor, so eine Nachricht
			 *  einfach zu loeschen, aber das ist (momentan) an
			 *  dieser Stelle nicht so einfach, daher setzen
			 *  wir erstmal einen Dummy-Absender ein.
			 */
			 if (!sp_name[0]) strcpy(sp_name, "nobody");
			 if (!sp_host[0]) strcpy(sp_host, "nowhere");
			 if (!sp_domain[0]) strcpy(sp_domain, "universe");
			 fprintf(f, HN_ABS": %s@%s.%s (Don't mail me!)\r\n", sp_name, sp_host,
		   		sp_domain);
		}
		if (wab_name[0] && (stricmp(sp_name, wab_name) != 0 || stricmp(sp_host, wab_host) != 0
		   || stricmp(sp_domain, wab_domain) != 0)) {
		/*
		 * WAB: der Form !User@system. darf nicht kommen.
		 * Einfach wegwerfen ist zwar keine gute Loesung,
		 * aber sonst kommen die Nachrichten gar nicht an.
		 *
		 * Diese WAB: entstehen bei Einlieferungen der Form
		 * @mail.other.do.main:USER@system
		 */
		 	if(wab_name[0] && (wab_name[0]!='!') && (wab_name[0]!='@')
		 		&& wab_host[0] && wab_domain[0])
		 	{
		   		fprintf(f, HN_WAB": %s@%s.%s\r\n", wab_name, wab_host,
		   			wab_domain);
		/*
		 * Wir haben einen WAB: geschrieben, also sollte gleich kein
		 * zweiter kommen...
		 */
		 		has_wab=1;
#ifndef DISABLE_NOUSER_IN_ROT
				sender = dstrdup( wab_name );
#endif
		   		hd = del_header(HD_UU_SENDER, hd);
		   	}
		}
		hd = del_header(HD_UU_FROM, hd);
	} else {
		if (convaddr(HN_ABS, from, 1, f) == 0) {
			 if (!sp_name[0]) strcpy(sp_name, "nobody");
			 if (!sp_host[0]) strcpy(sp_host, "nowhere");
			 if (!sp_domain[0]) strcpy(sp_domain, "universe");
			 fprintf(f, HN_ABS": %s@%s.%s (Don't mail me!)\r\n", sp_name, sp_host,
		   		sp_domain);
		}
	}
	p = find(HD_UU_SENDER, hd);
	if (p)
	{
	/*
	 * Sender: abc
	 * wandeln wir nicht in einen WAB:.
	 * Sender: mit zwei oder mehr @ auch nicht.
	 */
#ifndef DISABLE_NOUSER_IN_ROT
		sender = dstrdup( p->text );
#endif
		if(count(p->text,'@')==1 && 0==has_wab)
		{
			convaddr(HN_WAB, p->text, 1, f);
			has_wab=1;
			hd = del_header(HD_UU_SENDER, hd);
		}
	} else { /* eigentlich auch, wenn if(count... fehlgeschlagen ist, */
		p = find(HD_UU_RESENT_FROM, hd);
		if(p && 0==has_wab)
		{
			if(count(p->text,'@')==1)
			{
				convaddr(HN_WAB, p->text, 1, f);
				has_wab=1;
				hd = del_header(HD_UU_RESENT_FROM,hd);
			}
		}
	}
	p = find(HD_UU_SUBJECT, hd);
	if (p) {
	        char *subject = decode_mime_string(p->text);
		iso2pc(subject);
#ifdef ENABLE_NOEMPTY_SUBJECT
		if ( strlen( subject ) > 0 )
			fprintf(f, HN_BET": %s\r\n", subject);
		else
			fputs(HN_BET": -\r\n", f);
#else
		fprintf(f, HN_BET": %s\r\n", subject);
#endif
		dfree(subject);
		hd = del_header(HD_UU_SUBJECT, hd);
	} else
		fputs(HN_BET": -\r\n", f);
	p = find(HD_UU_DATE, hd);
	if (p) {
		char *datum;

		fprintf(f, HN_UU_U_DATE": %s\r\n", p->text);
#ifndef DISABLE_LOG_X_HEADER
		datum = date2eda( p->text, f );
#else
		datum = date2eda( p->text, NULL );
#endif
		fprintf(f, HN_EDA": %s\r\n", datum );
		hd = del_header(HD_UU_DATE, hd);
	}
	p = find(HD_UU_IN_REPLY_TO, hd);
	if (p) {
		s = strchr(p->text, '<');
		if (s) s = strchr(s, '>');
		if (s) {
			fprintf(f, HN_BEZ": ");
			/* Hier kommen wir nur hin, wenn es in p->text
			   '<'...'>' gibt. */
			for (s=strchr(p->text, '<')+1; *s && *s != '>'; s++)
				putc(*s, f);
			fputs("\r\n", f);
			hd = del_header(HD_UU_IN_REPLY_TO, hd);
		}
	}
	p = find(HD_UU_CONTROL, hd);
	if (p) {
		char *brett;
		fprintf(f, HN_UU_U_CONTROL": %s\r\n", p->text);
		fputs(HN_STAT": CTL\r\n", f);
		if(strncasecmp(p->text,"cancel ",strlen("cancel "))==0) {
			s=strchr(p->text, '<');
			if (s) {
				char *mid;
				int istda;
				mid = dstrdup(s+1);
				s = mid + strcspn(mid, "> ");
				if(s)
					*s = '\0';
				fprintf(f, HN_CONTROL": CANCEL %s\r\n",mid);
				/* OK. Ist die MID auch bei den
				 * References dabei, oder muessen wir
				 * den BEZ noch zusaetzlich erzeugen? */
				istda = 0;
				p = find(HD_UU_REFERENCES, hd);
				if(p)
					for(s = p->text; *s; s += strcspn(s,"<")) {
						if(*s=='<') s++;
						if(strncmp(mid,s,strcspn(s,"> "))==0) {
							istda=1;
							break;
						}
					}
				if(0==istda) /* Wir muessen selber... */
				fprintf(f, HN_BEZ": %s\r\n",mid);
				dfree(mid);
			}
		}
		else if (strncasecmp(p->text,"newgroup ", strlen("newgroup "))==0) {
			char *bname;

			fputs(HN_CONTROL": ADD ", f);
			brett = dstrdup(p->text + strlen("newgroup "));
			/* Flags (z.B. unmoderated) abschneiden */
			bname = brett;
			if ( *bname == ' ' )
				bname ++;
			s = strchr( bname, ' ');
			if ( s != NULL)
				*s = '\0';
			if ( strlen( bname ) > 0 )
				printbretter(bname, NULL, f);
			else
				fputs( "\r\n", f);
			dfree(brett);
		}
		else if (strncasecmp(p->text,"rmgroup ", strlen("rmgroup "))==0) {
			char *bname;

			fputs(HN_CONTROL": DEL ", f);
			brett = dstrdup(p->text + strlen("rmgroup "));
			/* Flags (z.B. unmoderated) abschneiden */
			bname = brett;
			if ( *bname == ' ' )
				bname ++;
			s = strchr( bname, ' ');
			if ( s != NULL)
				*s = '\0';
			if ( strlen( bname ) > 0 )
				printbretter(bname, NULL, f);
			else
				fputs( "\r\n", f);
			dfree(brett);
		}
		hd = del_header(HD_UU_CONTROL, hd);
	}
	p = find(HD_UU_REFERENCES, hd);
	if (p) {
		char buffer[MAXLINE], *t2;

		for (s=strchr(p->text, '<'); s && *s; s=strchr(s, '<')) {
			fputs(HN_BEZ": ", f);
			for (t2 = buffer, s++; *s && *s != '>'; s++) {
				putc(*s, f);
				*t2++ = *s;
			}
			*t2 = '\0';
			fputs("\r\n", f);
		}
		hd = del_header(HD_UU_REFERENCES, hd);
	}
	p = find(HD_UU_ORIG_TO, hd);
	if (p) {
	/* xxx //// mehrfach? */
		convaddr(HN_OEM, p->text, 0, f);
		hd = del_header(HD_UU_ORIG_TO, hd);
	}
	p = find(HD_UU_TO, hd);
	if (p) {
		to = decode_mime_string(p->text);
		iso2pc(to);
#ifndef DISABLE_UUCP_SERVER
		fprintf(f, HN_UU_U_TO": %s\r\n", to);
#endif
		if (!main_is_mail)
			convaddr(HN_KOP, p->text, 0, f);
		hd = del_header(HD_UU_TO, hd);
	}
	p = find(HD_UU_CC, hd);
	if (p) {
#ifndef DISABLE_UUCP_SERVER
	        char *kop=decode_mime_string(p->text);
		iso2pc(kop);
		fprintf(f, HN_UU_U_CC": %s\r\n", kop);
		dfree(kop);
#endif
		convaddr(HN_KOP, p->text, 0, f);
		hd = del_header(HD_UU_CC, hd);
	}
	p = find(HD_UU_BCC, hd);
	if (p) {
#ifndef DISABLE_UUCP_SERVER
	        char *bcc=decode_mime_string(p->text);
		iso2pc(bcc);
		fprintf(f, HN_UU_U_BCC": %s\r\n", bcc);
		dfree(bcc);
#endif
		convaddr(HN_KOP, p->text, 0, f);
		hd = del_header(HD_UU_BCC, hd);
	}
/*
 * Inzwischen wird der Routepfad direkt durchgereicht, um Probleme
 * nach dem Rueckkonvertieren zu vermeiden. Das ist zwar auf ZC-Seite
 * strenggenommen falsch, fuehrt aber zu den wenigsten Problemen.

 * Andererseits kann man damit den MAPS im Zerberus nicht verwenden.
 * Also gibt es die Moeglichkeit, Nachrichten an MAPS mit einem
 * ZC-konformen Routstring zu schreiben.

 * Nachdem wir dafuer aber meistens Nachrichten ohne Routstring (da vom
 * lokalen Host kommend) vorgesetzt bekommen werden, kopieren wir
 * den Routstring zunaechst in einen eigenen Speicherbereich und
 * arbeiten damit weiter.

 */
	{
		char *rot;
		p = find(HD_UU_PATH, hd);
		if (p) {
	 		rot = dstrdup(p->text);
#ifndef DISABLE_NOUSER_IN_ROT
			/* Schrittkette, keine Schleife */
			while (main_is_mail) {
				char *mailuser;
	  			mailuser = strrchr(rot, '!');
				if ( mailuser == NULL )
					break;
	  			s = strrchr(mailuser+1, '.');
				if ( s == NULL ) {
					/* Namen ohne '.' in Path sind User */
					*mailuser = 0;
					break;
				};
				/* User oder System ? */
				if ( sender == NULL )
					break;
				if ( stricmp( sender, mailuser+1 ) == 0 ) {
					*mailuser = 0;
					break;
				};
				break;
			};
#endif
			hd = del_header(HD_UU_PATH, hd);
		} else {
			rot = printpath(from);
		}

#ifndef DISABLE_ROT_FOR_MAPS
		/* Haben wir eine Mail vorliegen? Wenn ja, geht sie an maps? */
		if (to)
			if(strncasecmp(to, "maps@", sizeof("maps@")-1)==0) {
	  			s = strrchr(rot, '!');
	 			if (s) *s = '\0';
			}
#endif /* DISABLE_ROT_FOR_MAPS */

		fprintf(f, HN_ROT": %s\r\n", rot);
		dfree(rot);
	}
#ifndef DISABLE_NOUSER_IN_ROT
	dfree(sender);
#endif
	p = find(HD_UU_RETURN_RECEIPT_TO, hd);
	if (p) {
		convaddr(HN_EB, p->text, 0, f);
		hd = del_header(HD_UU_RETURN_RECEIPT_TO, hd);
	}
	p = find(HD_UU_ORGANIZATION, hd);
	if (p) {
	        char *org=decode_mime_string(p->text);
		iso2pc(org);
		fprintf (f, HN_ORG": %s\r\n", org);
		dfree(org);
		hd = del_header(HD_UU_ORGANIZATION, hd);
	}
        p = find(HD_UU_SUPERSEDES, hd);
        if (p) {
        	/* Patch von Moritz - Null Pointer sonst moeglich */
               	s=strchr(p->text, '<');
               	if (s) {
               	        fprintf(f, HN_ERSETZT": ");
               	        for (s++; *s && *s != '>' && *s != ' '; s++)
               	                putc(*s, f);
               	        putc('\r', f); putc('\n', f);
               	}
		hd = del_header(HD_UU_SUPERSEDES, hd);
	}
	p = find(HD_UU_EXPIRES, hd);
	if (p) {
		char *datum;

		fprintf(f, HN_UU_U_EXPIRES": %s\r\n", p->text);
#ifndef DISABLE_LOG_X_HEADER
		datum = date2eda( p->text, f );
#else
		datum = date2eda( p->text, NULL );
#endif
		fprintf(f, HN_LDA": %s\r\n", datum );
		hd = del_header(HD_UU_EXPIRES, hd);
	}
	p = find(HD_UU_REPLY_TO, hd);
	if (p) {
		convaddr(HN_ANTWORT_AN, p->text, 0, f);
		hd = del_header(HD_UU_REPLY_TO, hd);
	}
	p = find(HD_UU_X_MAILER, hd);
	if (p) {
	        char *mal=decode_mime_string(p->text);
		iso2pc(mal);
	        fprintf(f, HN_MAL": %s\r\n", mal);
		dfree(mal);
		hd = del_header(HD_UU_X_MAILER, hd);
	}
	p = find(HD_UU_LINES, hd);
	if (p) {
		fprintf(f, HN_UU_U_LINES": %s\r\n", p->text);
		hd = del_header(HD_UU_LINES, hd);
	}
	hd = del_header(HD_CONTENT_LENGTH, hd);

	p = find(HD_X_GATEWAY, hd);
	if (p) {
	  /* Sollte unnoetig sein. Wer weiss... */
	        char *gat=decode_mime_string(p->text);
		iso2pc(gat);
		fprintf(f, HN_GATE": RFC1036/822 %s.%s ["
			MAILER " " VERSION "], %s\r\n",
		   boxstat.boxname, boxstat.boxdomain, gat);
		dfree(gat);
		hd = del_header(HD_X_GATEWAY, hd);
	}else{
		fprintf(f, HN_GATE": RFC1036/822 %s.%s ["
			MAILER " " VERSION "]\r\n",
		   boxstat.boxname, boxstat.boxdomain);
	}

	/* Fido Brettempfaenger */
	p = find(HD_UU_X_COMMENT_TO, hd);
	if (p) {
	        char *fto=decode_mime_string(p->text);
		iso2pc(fto);
		fprintf(f, HN_F_TO": %s\r\n", fto);
		dfree(fto);
			hd = del_header(HD_UU_X_COMMENT_TO, hd);
		}

		/* eigentlich schoener waere es, diese Header oben zuerst auszugeben
		   und dann entsprechende Wandlungen zu unterbinden. */

		for (p = hd; p; p = t) {
			t=p->next;
			if (  strncasecmp(p->header, "X-ZC-", 5) == 0
			   || strncasecmp(p->header, "X-Z-", 6) == 0 ) {
				s=index(p->header,'-');
				if(NULL==s) continue;
				s=index(++s,'-');
				if(NULL==s) continue;
				s++;
				if ( strcasecmp(s, "WAB") == 0
				  || strcasecmp(s, "ABS") == 0
				  || strcasecmp(s, "ROT") == 0
				  || strcasecmp(s, "MID") == 0 ) {
					hd=del_header(p->code,hd);
			}
		}
	}

	for (p = hd; p; p = t) {
		if (strncasecmp(p->header, "X-ZC-", 5) == 0) {
			for (t = p; t; t = t->other) {
			  char *x = decode_mime_string(t->text);
			  iso2pc(x);
			  fprintf(f, "%s: %s\r\n", t->header + 5, x);
			  dfree(x);
			}
			t = p->next;
			hd = del_header(p->code, hd);
			continue;
		}
		if (strncasecmp(p->header, "X-Z-", 4) == 0) {
			for (t = p; t; t = t->other) {
			  char *x=decode_mime_string(t->text);
			  iso2pc(x);
			  fprintf(f, "%s: %s\r\n", t->header + 4, t->text);
			  dfree(x);
			}
			t = p->next;
			hd = del_header(p->code, hd);
			continue;
		}
		if (strncasecmp(p->header, "X-FTN-", 6) == 0) {
			for (t = p; t; t = t->other) {
			  char *x=decode_mime_string(t->text);
			  iso2pc(x);
			  fprintf(f, "F-%s: %s\r\n", t->header + 6, t->text);
			  dfree(x);
			}
			t = p->next;
			hd=del_header(p->code, hd);
			continue;
		}
#ifndef ENABLE_U_X_HEADER
		if (strncasecmp(p->header, "X-", 2) == 0) {
			for (t = p; t; t = t->other) {
			  char *x=decode_mime_string(t->text);
			  iso2pc(x);
			  fprintf(f, "%s: %s\r\n", t->header, t->text);
			  dfree(x);
			}
			t = p->next;
			hd = del_header(p->code, hd);
			continue;
		}
#endif
		t = p->next;
	}

	return hd;
}



/* Nachrichteninhalt und letzte Headerzeilen ausgeben je nach Typ und
 * Codierung
 */

int
make_body(char *bigbuffer, size_t msglen,
		mime_header_info_struct *mime_info,
		int binaer, char *smallbuffer, FILE *zconnect)
{
#ifdef APC_A2B
	size_t latob;
#endif

#ifdef APC_A2B
/* APC-typische Binaer-Nachrichten dekodieren. Wenn wir eine
 * solche haben, behandeln wir sie ganz separat von allem anderen.
 */
	latob = apc_a2b(bigbuffer, &msglen);
	if (latob > -1) {
		char *start = bigbuffer;
		fprintf(zconnect, HN_TYPE": BIN\r\n");
		/* ein CR/LF nehmen wir weg, es gehoeht noch zum
			RFC-Header */
		if (latob > 0 && *start == '\r') {
			start++; msglen--; latob--;
		}
		if (latob > 0 && *start == '\n') {
			start++; msglen--; latob--;
		}
		if (latob > 0) {
			/* Text, der der Nachricht vorangestellt ist */
			iso2pc_size(start, latob);
			fprintf(zconnect, HN_KOM": %ld\r\n", latob);
		}
		fprintf(zconnect, HN_LEN": %ld\r\n\r\n", msglen);
		fwrite(start, (size_t) msglen, 1, zconnect);
	}
	else
#endif /* APC_A2B */

	/* Hier kommt die UnixConnect-Spezial-Multipart-Abteilung.
	 * Es handelt sich um ehemalige binaere ZConnect-Nachrichten, die
	 * einen KOM:-Header hatten.
	 * Wir machen das nur, wenn sie oben nicht MIME-Dekodiert wurde,
	 * damit wir sicher sind, dass wir am Ende der Nachricht (und
	 * erst dort) ein Nullbyte haben. Dann koennen wir strchr()
	 * und dergleichen benutzen.
	 */

	  /* Das hier sollte noch Gatebau97-konform umgebaut werden. xxx */
	if (!binaer && mime_info->unixconnect_multipart) {
		char *p1, *p2;
		size_t wrlen;

		wrlen=msglen;
		/* Erste border nach p1 */
		p1 = strstr(bigbuffer, "--"SP_MULTIPART_BOUNDARY);
		if (p1)
			/* LF vor Border zw. KOM: und Binaerdaten nach p2 */
			p2 = strstr(p1+5, "\n--"SP_MULTIPART_BOUNDARY);
		else
			p2 = NULL;
		if (p1 && p2) {
			int i;

			/* boundary, maximal 2 Headerzeilen
			 * und Leerzeile ueberspringen
			 */
			for (i=4; i>0; i--) {
				while (*p1 && *p1 != '\n')
					p1++;
				p1++;
				if ((*p1 == '\r' || *p1 == '\n')
					&& i>2)
					i = 2; /* Leerzeile noch ueberspringen */
			}
			/* KOM: Text ist jetzt p1 bis p2 (incl.!) */
			msglen = p2-p1+1;
			memcpy(smallbuffer, p1, msglen);
			/* CHARSET gilt fuer KOM:-Teil nicht */
			iso2pc_size(smallbuffer, msglen);
			fprintf (zconnect, HN_KOM": %d\r\n", msglen);

			/* Ende der Boaderzeile */
			while (*p2 && *p2 != '\n')
				p2++;
			p2++;
			/* Content-Type */
			while (*p2 && *p2 != '\n')
				p2++;
			p2++;
			p1 = strstr(p2, "--"SP_MULTIPART_BOUNDARY);
			if (p1)
				wrlen = p1 - p2;
			else
				wrlen -= (p2 - bigbuffer);
			/* Na, dann versuchen wir mal zu decodieren */
			if (decode_x_uuencode(p2, &wrlen, mime_info))
				fprintf(zconnect, HN_TYPE": BIN\r\n");
		}
		else
			wrlen = 0;
		if (mime_info->filename)
			fprintf(zconnect, HN_FIL": %s\r\n", mime_info->filename);
		fprintf(zconnect, HN_LEN": %d\r\n\r\n", msglen+wrlen);
		fwrite(smallbuffer, msglen, 1, zconnect);
		fwrite(p2, wrlen, 1, zconnect);
	}
	else
	if (!binaer) {
		char *start;

		start = bigbuffer;

/*
 * Leerzeilen am Anfang wegzuwerfen ist nicht nur schlechter Stil,
 * sondern auch noch unerlaubt und fuehrt dazu, dass der Lines:-Header
 * nach einer RFC->ZC->RFC-Konvertierung evtl. nicht mehr stimmt.

		if (msglen && *start == '\r') {
			start++;
			msglen--;
		}
		if (msglen && *start == '\n') {
			start++;
			msglen--;
		}
*/
		/* Zeichensatz-Konvertierung des Body nur, wenn
		 * es sich um text/plain handelt (default)
		 * (ist auch bei nicht-MIME-Nachrichten gesetzt)
		 */
		 if (mime_info->text_plain) {
#ifndef ENABLE_ISO_IN_ZCONNECT
			/* Bei Zeichensaetzen us-ascii,
			 * undefiniert oder iso-8859-1
			 * konvertieren wir nach IBM */
			if (mime_info->charset > 1 )
				fprintf(zconnect, HN_CHARSET": ISO%d\r\n", mime_info->charset);
			else
				iso2pc_size(start, msglen);
#else /* ENABLE_ISO_IN_ZCONNECT */
			/* Wir konvertieren nie nach IBM,
			 * setzen aber eine CHARSET:
			 * Headerzeile (default: ISO1)
			 */
			fprintf(zconnect, HN_CHARSET": ISO%d\r\n",
				mime_info->charset >0 ? mime_info->charset : 1);
#endif /* ENABLE_ISO_IN_ZCONNECT */
		}
		fprintf (zconnect, HN_LEN": %d\r\n\r\n", msglen);

		/* Inhalt ausgeben: */
		fwrite(start, (size_t)msglen, 1, zconnect);
	} else {
		/* Binaer */
		fprintf(zconnect, HN_TYPE": BIN\r\n");
		if (mime_info->filename) {
			fprintf(zconnect, HN_FIL": %s\r\n", mime_info->filename);
		}
		fprintf(zconnect, HN_LEN": %d\r\n\r\n", msglen);
		fwrite (bigbuffer, 1, msglen, zconnect);
	}
	return 0;
}
