/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999       Matthias Andree
 *  Copyright (C) 2000       Krischan Jodies
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
#include "idir.h"
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "utility.h"
#include "xprog.h"

#include "calllib.h"
#include "uudebug.h"

/*
 *  Alle Dateien im aktuellen Verzeichnis entpacken und einlesen...
 */
int import_all(const char *arcer, const char *sysname)
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
				backup3(backindir,l->name,sysname,arcer);
			} else {
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

