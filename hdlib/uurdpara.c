/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
 *  Copyright (C) 1999     Dirk Meyer
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


/********************************************************************
 *
 *   File    :  uurdpara.c
 *   Autoren :  Martin
 *
 *              Routinen zum Einlesen von UUCP-Headern aus
 *              News- oder SMTP-Batchen.
 *
 *              Compiliert mit "#define SMTP" wird die SMTP-Version
 *              erzeugt, ansonsten die News-Version.
 *
 *              Unterschied: SMTP markiert das Ende der Eingabe mit "\r\n.\r\n",
 *              im Newsbatch ist die LÑnge der News vorher bekannt. Au·erdem
 *              ist im Newsbatch Åblicherweise nur "\n" als Zeilenende,
 *              im SMTP-Batch aber "\r\n".
 */

# include "config.h"
# include <stdio.h>
# include <stdlib.h>
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
# include <ctype.h>

# include "header.h"
# include "uulog.h"
# include "lib.h"

static char *uugets(char *s, int n, FILE *stream);

#ifndef SMTP

extern long readlength;		/* Wird auf 0 heruntergezÑhlt */
#define	COUNTDOWN(count)	readlength -= (count)
#define CHECKCOUNT(action)	if (readlength<1) action

/********************************************************************
 *
 *   Funktion:  header_p uu_rd_para(FILE *input)
 *
 *              Liest einen UUCP-Header aus einem News-Batch.
 *              Die globale Variable "readlength" (long) wird
 *              um die gelesene Anzahl von Bytes reduziert.
 */

header_p uu_rd_para(FILE *f)

#else /* jetzt kommt die SMTP-Version */

#define	COUNTDOWN(count)	/* - */
#define CHECKCOUNT(action)	/* - */

/********************************************************************
 *
 *   Funktion:  header_p smtp_rd_para(FILE *input)
 *              Liest einen UUCP-Header aus einem SMTP-Batch.
 */

header_p smtp_rd_para(FILE *f)

#endif /* SMTP */

{
        header_p start;
        char c, *p, *str1, *str2, *inhalt;
	char buffer[MAX_HEADER_NAME_LEN+1], hname[MAX_HEADER_NAME_LEN+1];
        int was_malloc;
	unsigned code;

        start = NULL;
	while(!feof(f)) {

	      CHECKCOUNT(break);
	      /* naechste Header-Zeile holen */
	      if (!uugets(hname, MAX_HEADER_NAME_LEN, f)) break;
	      hname[MAX_HEADER_NAME_LEN] = '\0'; /* Sentinel */

              if ((hname[0] == '\r') || (hname[0] == '\n')) {
                  if (start)
                     break;     /* Ende einer Header-Liste */
                  else
                     continue;  /* Leerzeile am Anfang einer Header-Liste */
	      }

              /* Header-Name und Inhalt trennen */
              p = strchr(hname, ':');
              if (!p)
              {
                  uufatal(__FILE__, "Header ohne Doppelpunkt: %s", hname);
              }
	      *p++ = '\0'; /* p zeigt auf Anfang des Headers. */

              /* Syntax prÅfen */
              for (str1=hname; *str1; str1++) {
                 if (!isascii(*str1))
                 {
                    uufatal(__FILE__,"Nicht-ASCII-Zeichen im Header: %s", hname);
                 }
                 if (isspace(*str1))
                 {
                    uufatal(__FILE__, "Leerzeichen im Headernamen: %s", hname);
                 }
              }

	      /* Header identifiziern */
	      code = identify(hname);

	      /* Inhalt einlesen und aufbereiten */
	      c = '\0';
	      while (1) {
		while (*p)
		{
		      /* Anfang des Textes suchen */
		      c = *p;
		      if ((c == '\r') || (c == '\n')) break;
		      if (!isspace(c)) break;
		      p++;
		} /* Leerzeichen am Anfang nicht zwingend. */

		if (*p == '\0') {	/* Der Inhalt hat hier noch gar nicht begonnen */
			CHECKCOUNT(break);
			if (!uugets(buffer, MAX_HEADER_NAME_LEN, f)) break;
			buffer[MAX_HEADER_NAME_LEN] = '\0';
			p = buffer;
		} else
			break;
	      }
	      if ((c == '\r') || (c == '\n')) {
		      inhalt = dstrdup( "" );
		      was_malloc = 1;
	      } else {
		      inhalt = p;
		      was_malloc = 0;

		      /* Der Header ist beim 1. CR oder 1. LF zuende */
		      str1 = strchr(inhalt, '\r');
		      if (str1) *str1 = '\0';
		      str2 = strchr(inhalt, '\n');
                      if (str2) *str2 = '\0';

                      /* Wenn weder \n noch \r gefunden werden, dann ist der
                       * Header lÑnger als MAX_HEADER_NAME_LEN Bytes. */
		      if (!str1 && !str2) {
				while (1) {
					str1 = malloc(strlen(inhalt) + MAX_HEADER_NAME_LEN + 1);
					if (!str1)
						out_of_memory(__FILE__);
					strcpy(str1, inhalt);
					if (was_malloc) {
						dfree(inhalt);
					} else {
						was_malloc = 1;
					}
					inhalt = str1;
					CHECKCOUNT(break);
					if (!uugets(buffer, MAX_HEADER_NAME_LEN, f)) break;
					buffer[MAX_HEADER_NAME_LEN] = '\0';
					strcat(inhalt, buffer);
					str1 = strchr(inhalt, '\r');
					if (str1) {
						*str1 = '\0';
						break;
					}
					str2 = strchr(inhalt, '\n');
					if (str2) {
						*str2 = '\0';
						break;
					}
				}
		      }
              }
              /* Header-Descriptor erzeugen und einbauen */
	      start = internal_add_header(inhalt, code, start, hname);
	      if (was_malloc) dfree(inhalt);
              was_malloc = 0;
	}
	return start;
}

static char *uugets(char *s, int n, FILE *stream)
{
	static int stop = 0;
	int c, last;
	char *p;

	if (stop) {
		*s = '\n';
		*(s+1) = '\0';
		stop = 0;
		return s;
	}
	p = s;
	while (0 < n) {
		CHECKCOUNT(break);
		c = getc(stream);
		COUNTDOWN(1);
		if (c == EOF) return NULL;
		if (c == '\n'
#ifdef SMTP
			 || c == '\r'
#endif
		   ) {
			CHECKCOUNT(break);
			last = c;
			c = getc(stream);
			if (last == '\r' && c == '\n') {
				COUNTDOWN(1);
				c = getc(stream);
			}
			if (
#ifdef SMTP
			   c == '\r' ||
#endif
			   c == '\n') {
				*p++ = '\n';
				*p = '\0';
				stop = 1;
				COUNTDOWN(1);
				return s;
			} else if (!isspace(c)) {
				ungetc(c, stream);
				*p++ = '\n';
				*p = '\0';
				stop = 0;
				return s;
			}
			COUNTDOWN(1);
		}
		*p++ = c;
		n--;
	}
	*p = '\0';
	return s;
}



