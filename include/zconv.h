/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999-2000  Dirk Meyer
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

#define	MAXLINE		8192		/* Z.B. in einem SMTP Batch */

char *eda2date(const char *text );
int valid_newsgroups( const char *data );
int adrmatch(const char *abs1, const char *abs2);
int printnewsgroup(const char *brett, FILE *f);
void printnewsgroups(header_p p, const char *uuheader, FILE *f);
header_p convheader(header_p hd, FILE *f);
void u_f_and_all(FILE *f, header_p hd);
void foldputs(FILE *, const char *, const char *);
void foldputh(FILE *f, const char *hd, header_p t);
void foldputaddrs(FILE *f, const char *hd, header_p t);

