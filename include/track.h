/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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
 *  christopher@nescio.foebud.org or snail-mail:
 *  Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

/*
 *   track.h
 *
 *   Header f�r track.c
 *
 *   track.c implementiert einen Knuth-Morris-Pratt Algorithmus
 *   zur simultanen Patternsuche in mehreren String ohne Backup.
 */

#ifndef SYSDEP_H
#include "sysdep.h"
#endif


# define MAXPATTERN    256 /* L�nger darf der Suchbegriff nicht sein */
# define MAXTRACKS	16 /* Mehr Strings k�nnen wir nicht verwalten */

/* init_track initialisiert den Such-Automaten und �bergibt das      */
/* zu suchende Pattern.                                              */
/* R�ckgabewert ist die interne Nummer des patterns, die bei einem   */
/* Treffer von track_char zur�ckgeliefert wird.			     */
int init_track(const char *pattern);

/* track_char �bergibt das n�chste Zeichen.                          */
/* R�ckgabewert ist -1 : wenn das Pattern nocht nicht gefunden wurde */
/*		  >= 0 : die Nummer des gefundenen Patterns	     */
int track_char(const char input);

/* free_track entfernt das pattern aus der internen Suchstruktur und */
/* gibt diesen Eintrag wieder frei				     */
void free_track(const int patnr);
void free_all_tracks(void);
