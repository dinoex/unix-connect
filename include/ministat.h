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
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

typedef struct str_list_st {
	char *text;
	struct str_list_st *next;
} node_t, *list_t;

extern char *ortsnetz;
extern char *fernwahl;
extern char *international;
extern char *logdir;
extern char *lockdir;
extern char *netcalldir;
extern char *backindir;
extern char *backoutdir;
extern char *backupnumber;
extern char *systemedir;
extern char *fileserverdir;
extern char *fileserveruploads;
extern char *myself;
extern char *path_qualifier;
extern char *default_path;
extern char *autoeintrag;
extern char *einpack;
extern char *auspack;
extern char *import_prog;
extern char *uucpspool;
extern char *aliasliste;
extern char *approvedliste;
extern char *inewscmd;
extern int  debuglevel;
#ifdef MAX_HEADER_NAME_LEN
extern header_p config;
#endif
extern list_t local_domains;

