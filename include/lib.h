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

char *strupr(char *a);
char *strlwr(char *a);
int stricmp(const char *a, const char *b);
int lock_device (int flok, const char *device);

char *dstrdup(const char *);
void *dalloc(size_t);

void iso2pc(unsigned char *buffer, size_t len);
void pc2iso(unsigned char *buffer, size_t len);

#ifdef __GNUC__
static inline void to_pc (unsigned char *) __attribute__ ((unused));
static inline void to_iso(unsigned char *) __attribute__ ((unused));
#endif

static inline void to_pc(unsigned char *buf) { iso2pc(buf, strlen(buf)); }
static inline void to_iso(unsigned char *buf) { pc2iso(buf, strlen(buf)); }

void splitaddr(char *rfc_addr, char *name, char *host, char *domain,
	char *realname);

void ulputs(char *text, FILE *f);

/*
 *  Prüfe ein lock-File und warte gegebenenfalls, bis es verschwindet.
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
