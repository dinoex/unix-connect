/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
 *  Copyright (C) 1999     Dirk Meyer
 *  Copyright (C) 1999     Matthias Andree
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
 *   import.c:
 *
 *	Liest alle Daten im aktuellen Verzeichnis ein (und packt sie vorher
 *	aus.
 */


#include "config.h"
#include "zconnect.h"

#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "utility.h"
#include "xprog.h"

#include "calllib.h"
#include "uudebug.h"

#ifdef ENABLE_TESTING

/*
 *  Alle Dateien im aktuellen Verzeichnis entpacken und einlesen...
 */
int import_all(char *arcer, char *sysname)
{
	int returncode = 0;

	/* Liste der zu entpackenden Dateien */
	ilist_p l;

	logcwd("import_all");
	l = readonedir(".");

  	while (l) {
  		ilist_p p;
		int myret = 1;
		struct stat st;

		if(stat(l->name, &st)) {
			newlog(ERRLOG, "Kann nicht auspacken: %s: %s", l->name,
			       strerror(errno));
			perror(l->name);
		} else {
			newlog(DEBUGLOG, "Auspacken: %s (%s), %ld bytes\n",
				l->name, arcer, (long)st.st_size);
			fprintf(stderr, "Auspacken: %s (%s), %ld bytes\n",
				l->name, arcer, (long)st.st_size);
			if (S_ISREG(st.st_mode)
			    && (nlink_t)1==st.st_nlink) {
				if(!call_auspack(arcer, l->name)) {
					myret = 0;
				}
				if(backindir) {
					backup(backindir, l->name,
					       sysname, BACKUP_MOVE);
				} else {
					remove(l->name);
				}
			} else {
				fprintf(stderr,
					"File hat falschen Link count "
					"oder ist kein regulaeres File!\n");
				newlog(ERRLOG,
					"File hat falschen Link count "
					"oder ist kein regulaeres File!\n");
			}
		}
		returncode |= myret;
		p = l; l = p->next;
		dfree(p->name); dfree(p);
	}

	returncode |= call_import(sysname);
	return returncode;
}

#else

/*
 *  Alle Dateien im aktuellen Verzeichnis entpacken und einlesen...
 */
int import_all(char *arcer, char *sysname)
{
	int returncode = 0;

	DIR *dir;
	struct dirent *ent;
	ilist_p l;		/* Liste der zu entpackenden Dateien */

	if ((dir = opendir(".")) == NULL) {
		perror(".");
	}
	l = NULL;
	while ((ent = readdir(dir)) != NULL) {
		ilist_p neu;

		/* nur . und .. ignorieren */
		if (ent->d_name[0] == '.') {
			if (!ent -> d_name[1]) continue;
			if (ent->d_name[1] == '.' 
			    && !ent->d_name[2]) continue;
		}
		neu = dalloc(sizeof(ilist_t));
		neu->name = dstrdup(ent->d_name);
		neu->next = l;
		l = neu;
	}
	closedir(dir);

  	while (l) {
  		ilist_p p;
		int rc;
		int myret = 1;
		struct stat st;

		fprintf(stderr, "Auspacken: %s (%s)\n", l->name, arcer);
		fflush(stderr);
		if(lstat(l->name, &st)) {
			perror(l->name);
		} else {
			if (S_ISREG(st.st_mode) && (nlink_t)1==st.st_nlink) {
			    rc = call_auspack(arcer, l->name);
			    if (!rc) {
				myret = 0;
				    if(backindir) {
					char backinname[FILENAME_MAX];
					char *shortname;
						
					shortname = strrchr(l->name, '/');
					if (!shortname) shortname = l->name;
					sprintf(backinname, "%s/%s.%s.%ld",
						backindir, sysname,
						shortname,
						(long)time(NULL));
					fprintf(stderr, "BackIn: %s -> %s\n",
						l->name, backinname);
					fflush(stderr);
					remove(backinname);
					rename(l->name, backinname);
				    } else {
					remove(l->name);
				}
			    }
			} else {
			   fprintf(stderr,
				"File hat falschen Link count "
				"oder ist kein regulaeres File!\n");
			}
		}
		returncode |= myret;
		p = l; l = p->next;
		dfree(p->name); dfree(p);
	}

	returncode |= call_import(sysname);
	return returncode;
}

#endif
