/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
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


/*
 *  list-systems: listet alle Systeme auf, zu denen eine Direktverbindung
 *                besteht
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include "lib.h"
#ifdef HAS_BSD_DIRECT
#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/dir.h>
#else
#include <dirent.h>
#endif

#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "ministat.h"

void usage(void);
void usage(void)
{
	fputs("list-systems: Listet alle ZCONNECT/JANUS Systeme,\n"
	      "zu denen eine Direktverbindung aufgebaut werden kann.\n"
	      "Optionen:\n"
	      "   -p    Pathalias-Modus: erzeugt eine Paths-Datei fuer smail\n",
	     stderr);
	exit(1);
}

int main(int argc, char **argv)
{
	DIR *dir;
#ifdef HAS_BSD_DIRECT
	struct direct *ent;
#else
	struct dirent *ent;
#endif
	FILE *f;
	char name[FILENAME_MAX], sys[100], rot[100], *s;
	header_p hd, p;
	int dopaths = 0;

	if (argc != 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
		} else if (strcmp(argv[1], "-p") == 0) {
			dopaths = 1;
			fprintf(stderr, "Erzeuge Routing-Tabelle...\n");
		} else
			usage();
	}
	minireadstat();
	if ((dir = opendir(systemedir)) == NULL)
	{
		perror(systemedir);
		exit(1);
	}
	while ((ent = readdir(dir)) != NULL) {
		if(*(ent->d_name) != '.') {
			sprintf(name, "%s/%s", systemedir, ent->d_name);
			f = fopen(name, "r");
			if (!f) continue;
			hd = rd_para(f);
			fclose(f);
			p = find(HD_SYS, hd);
			if (!p) {
				free_para(hd);
				continue;
			}
			strncpy(sys, p->text, sizeof sys -1);
			sys[sizeof sys -1] = '\0';
			strlwr(sys);
			p = find(HD_DOMAIN, hd);
			if (!p) {
				free_para(hd);
				continue;
			}
			strncpy(name, p->text, FILENAME_MAX-1);
			name[FILENAME_MAX-1] = '\0';
			strlwr(name);
			rot[0] = '\0';
			for (s = name; (s = strtok(s, " ;,")) != NULL; s = NULL) {
				if (dopaths) {
					if (!rot[0])
						sprintf(rot, "%s.%s", sys, s);
					printf("%s.%s\t%s!%s.%s!%%s\n", sys, s, rot, sys, s);
				} else
					printf("%s.%s\n", sys, s);
			}
			free_para(hd);
		}
	}

	if (closedir(dir) != 0)
		perror(systemedir);

	return 0;
}
