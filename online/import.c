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
 *   unix-connect-request@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */


/*
 *  ZCONNECT fuer UNIX, (C) 1993 Martin Husemann, (C) 1995 Christopher Creutzig
 *
 *  Sie sollten mit diesen Quelltexten eine Kopie der GNU General Public
 *  Licence erhalten haben (Datei COPYING).
 *
 *  Diese Software ist urheberrechtlich geschuetzt. Sie ist nicht und war
 *  auch niemals in der Public Domain. Alle Rechte vorbehalten.
 *
 *  Sie duerfen diese Quelltexte entsprechend den Regelungen der GNU GPL
 *  frei nutzen.
 *
 * ---------------------------------------------------------------------------
 *
 *   import.c:
 *
 *	Liest alle Daten im aktuellen Verzeichnis ein (und packt sie vorher
 *	aus.
 *
 * ---------------------------------------------------------------------------
 *  Letzte Aenderung: martin@isdn.owl.de.owl.de, Tue Aug 17 15:07:22 1993
 */


#include "config.h"
#include "zconnect.h"
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAS_BSD_DIRECT
#include <sys/dirent.h>
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#ifdef HAS_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#include "lib.h"
#include "xprog.h"

/*
 *  Alle Dateien im aktuellen Verzeichnis entpacken und einlesen...
 */
void import_all(char *arcer, char *sysname, int ist_net38)
{
	typedef struct list_st {
		char *name;
		struct list_st *next;
	} list_t, *list_p;
	
	DIR *dir;
#ifdef HAS_BSD_DIRECT
	struct direct *ent;
#else
	struct dirent *ent;
#endif
	list_p l;		/* Liste der zu entpackenden Dateien */

	if ((dir = opendir(".")) == NULL) {
		perror(".");
	}
	l = NULL;
	while ((ent = readdir(dir)) != NULL) {
		list_p neu;
		
		if (*(ent->d_name) == '.') continue;
		neu = dalloc(sizeof(list_t));
		neu->name = dstrdup(ent->d_name);
		neu->next = l;
		l = neu;
	}
	closedir(dir);

	while (l) {
		list_p p;

		fprintf(stderr, "Auspacken: %s (%s)\n", l->name, arcer);
		fflush(stderr);
		call_auspack(arcer, l->name);
		if (backindir) {
			char backinname[FILENAME_MAX], *shortname;

			shortname = strrchr(l->name, '/');
			if (!shortname) shortname = l->name;
			sprintf(backinname, "%s/%s.%s.%ld", backindir, sysname, shortname, time(NULL));
			fprintf(stderr, "BackIn: %s -> %s\n", l->name, backinname);
			fflush(stderr);
			remove(backinname);
			rename(l->name, backinname);
		} else {
			remove(l->name);
		}
		p = l; l = p->next;
		dfree(p->name); dfree(p);
	}

	call_import(sysname, ist_net38);
}
