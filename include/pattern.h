/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
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


/*
 *   pattern.h
 *
 *   Header fuer pattern.c
 *
 *   Pattern.c implementiert einen Knuth-Morris-Pratt Algorithmus
 *   zur Patternsuche in einem String ohne Backup.
 */

# define MAXPATTERN    256 /* Laenger darf der Suchbegriff nicht sein */

/* init_search initialisiert den Such-Automaten und uebergibt das    */
/* zu suchende Pattern.                                              */
int init_search(const char *pattern);

/* search_char uebergibt das naechste Zeichen.                        */
/* Rueckgabewert ist  0 - wenn das Pattern nocht nicht gefunden wurde */
/*                    1 - wenn das letzte Zeichen das Pattern         */
/*                                                  vervollstaendigte */
int search_char(const char input);

