/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
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
 * readonedir.c
 *
 * baut eine verkettete Liste der Dateinamen im Directory
 * laesst . und .. aus
 *
 */

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
#include "istring.h"

#include "utility.h"
#include "calllib.h"
#include "uulog.h"

ilist_p
readonedir(const char *name)
{
	ilist_p l = NULL;
	DIR *dir;
	struct dirent *ent;

	if (!(dir = opendir(name))) {
		newlog(ERRLOG, "cannot open directory %s: %s",
		       name, strerror(errno));
	} else {
		errno = 0;
		while ((ent = readdir(dir)) != NULL) {
			ilist_p neu;

			/* . und .. ignorieren */
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
		if(errno) {
			newlog(ERRLOG, "cannot read directory: %s",
			       strerror(errno));
		}
		closedir(dir);
	}
	return l;
}
