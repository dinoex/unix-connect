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
 *   File    :  rdpara.c
 *   Autoren :  Martin
 *   Funktion:  Liest einen Header
 */

#include "config.h"

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

#include "utility.h"
#include "crc.h"
#include "header.h"
#include "uulog.h"

#ifndef DO_CRC
static int may_i_call_fatal = 1;

int rd_para_error = HEAD_NO_ERROR;

int
set_rd_para_reaction(int new_reaction)
{
   int old_reaction = may_i_call_fatal;
   if (new_reaction >= 0) may_i_call_fatal = new_reaction;
   return old_reaction;
}
#else
const int may_i_call_fatal = 0;
#endif

#ifdef DO_CRC

static crc_t crc;

/*
 * Wie fgets, aber gleichzeitig CRC updaten und Sonderzeichen streichen.
 * int name == 0 : hier kann kein Header-Name stehen (wir sind weiter hinten
 *                 in der Zeile)
 * int name == 1 : Wenn der eingelesene Text mit CRC: beginnt, crc nicht
 *                 berechnen, bis zum naechsten name == 1
 */
static char *
crc_gets(char *buffer, size_t max_len, FILE *f, int name)
{
	unsigned char *p;
	int z;

	if (max_len <= 1)
		return NULL;

	for (p = (unsigned char *)buffer; 0 < max_len; ) {
		do {
			z = getc(f);		/* wiederholen, bis genau 1 */
			if (z == EOF && feof(f))/* Zeichen gelesen wurde */
				break;		/* ausser bei EOF -> Abbruch */
			clearerr(f);
		} while (z == EOF);
		if (z == EOF) break;		/* Ganz abbrechen */
		if (z == '\r') {
			*p++ = '\n';
			break;
		}
		if (z < ' ' || z > 127)
			continue;
		*p++ = (unsigned char)z;
		max_len--;
	}

	if (p == (unsigned char *) buffer)
		return NULL;

	*p = '\0';
	if (!name || toupper(buffer[0]) != 'C' ||
	   toupper(buffer[1]) != 'R' ||
	   toupper(buffer[2]) != 'C' ||
	   buffer[3] != ':') {
		for (p=(unsigned char *)buffer; *p && *p != '\n'; p++) {
			crc = CRC(*p, crc);
		}
	}
	return buffer;
}

header_p
rd_packet(FILE *f, crc_t *erg_crc)

#else

/*
 *  Version ohne CRC:
 */

#define	crc_gets(b,l,f,n)	fgets(b,l,f)

header_p
rd_para(FILE *f)

#endif

{
        header_p start;
        char c, *p, *str1, *str2, *inhalt;
	char buffer[MAX_HEADER_NAME_LEN+1], hname[MAX_HEADER_NAME_LEN+1];
        int was_malloc;
	unsigned code;

        rd_para_error = HEAD_NO_ERROR;
        start = NULL;
#ifdef DO_CRC
        crc = ~0;
#endif
	while(!feof(f)) {

	      /* naechste Header-Zeile holen */
	      if (!crc_gets(hname, MAX_HEADER_NAME_LEN, f, 1)) break;
              if (hname[0] == '#' || hname[0] == ' ') {
		/* Kommentarzeile ueberlesen... */
		while (1) {
			p = strchr(hname, '\n');
			if (p) break;
			p = strchr(hname, '\r');
			if (p) break;
			if (!crc_gets(hname, MAX_HEADER_NAME_LEN, f, 0)) break;
		}
		continue;
	      }
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
                  if (may_i_call_fatal)
                     uufatal(__FILE__, "Header ohne Doppelpunkt: %s", hname);
                  if (start) free_para (start);
                  rd_para_error = HEAD_ILLEGAL_HEADER;
                  return NULL;
              }
	      *p++ = '\0'; /* p zeigt auf Anfang des Headers. */

              /* Syntax pruefen */
              for (str1=hname; *str1; str1++) {
                 if (!isascii(*str1))
                 {
                    if (may_i_call_fatal)
                       uufatal(__FILE__, "Nicht-ASCII-Zeichen im Header: %s", hname);
                    if (start) free_para (start);
                    rd_para_error = HEAD_ILLEGAL_HEADER;
                    return NULL;
                 }
                 if (isspace(*str1))
                 {
                    if (may_i_call_fatal)
                       uufatal(__FILE__, "Leerzeichen im Headernamen: %s", hname);
                    if (start) free_para (start);
                    rd_para_error = HEAD_ILLEGAL_HEADER;
                    return NULL;
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
			if (!crc_gets(buffer, MAX_HEADER_NAME_LEN, f, 0)) break;
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
                       * Header laenger als MAX_HEADER_NAME_LEN Bytes. */
		      if (!str1 && !str2) {
				while (1) {
					str1 = dalloc(strlen(inhalt) + MAX_HEADER_NAME_LEN + 1);
					strcpy(str1, inhalt);
					if (was_malloc) {
						dfree(inhalt);
					} else {
						was_malloc = 1;
					}
					inhalt = str1;
					if (!crc_gets(buffer, MAX_HEADER_NAME_LEN, f, 0)) break;
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
#ifdef DO_CRC
	*erg_crc = crc;
#endif
	return start;
}

