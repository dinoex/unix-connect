/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999-2000  Dirk Meyer
 *  Copyright (C) 1999-2000  Matthias Andree
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
 *  utility.h
 *
 *  Lib-Funktionen fuer RFC/Connect
 */

void * dalloc(size_t);
char * dstrdup(const char *);
char * str2dup(const char *);

int stricmp(const char *a, const char *b);
char * strlwr(char *);
char * strupr(char *);

#ifndef HAVE_STCCPY
size_t stccpy(char *, const char *, size_t);
#endif
void qstccpy(char *, const char *, size_t);
char * strcdup(const char *, size_t);

/*
 *  Pruefe ein lock-File und warte gegebenenfalls, bis es verschwindet.
 *  'ltimeout' gibt die Warte-Strategie an:
 *
 *	< 0 :	warte maximal abs(ltimeout) Sekunden und versuch dann, den
 *		Lock zu entfernen
 *	  0 :	warte zur Not ewig
 *	> 0 :	warte maximal timeout Sekunden und gib dann auf
 *
 *   Returns:	0 bei Erfolg
 */
int waitnolock(const char *lockname, long ltimeout);

#define GET_NEXT_DATA(x)	{ argv++; argc--; x = *argv;	\
				if (x == NULL) usage(); }

#define dfree(x)	{ free(x); (x) = NULL; }

