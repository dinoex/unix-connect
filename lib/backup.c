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
 *  Dirk Meyer, Im Grund 4, 34317 Habichstwald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

/*
 * backup.c
 *
 * Backup eines Puffers anlegen
 *
 */

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
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#include <stdio.h>

#include "utility.h"
#include "calllib.h"
#include "uulog.h"

/* backup
 * inputs:
 *
 *
 * returns:
 * 0: okay
 * 1: error during execution
 * 2: not a regular file
 * 3: file not found
 * -1: file, backup or sysname have been NULL or an illegal backup_type
 *     has been specified
 */
int
backup(const char *backupdir, const char *file,
       const char *sysname, enum backup_type backup_type)
{
	logcwd("backup");

	if (backupdir && file && sysname) {
		char backupname[FILENAME_MAX];
		const char *shortname, *btype;
		int rc;
		struct stat st;

		if(lstat(file, &st)) {
			newlog(ERRLOG, "backup: cannot stat %s: %s",
			       file, strerror(errno));
			return 3;
		}
		if(!S_ISREG(st.st_mode)) {
			newlog(ERRLOG,
			       "backup: %s is not a regular file, mode: 0%o",
			       file, st.st_mode);
			return 2;
		}

		shortname = strrchr(file, '/');
		if (!shortname) shortname = file;
		else shortname++;
		snprintf(backupname, sizeof(backupname),
			 "%s/%s.%s.%ld",
			 backupdir, sysname,
			 shortname, (long)time(NULL));
		newlog(DEBUGLOG, "Backup: %s -> %s\n", file, backupname);
		if(remove(backupname) && errno != ENOENT) {
			newlog(ERRLOG,
			       "cannot remove backup destination %s: %s",
			       backupname, strerror(errno));
			return 1;
		}

		switch(backup_type) {
		case BACKUP_MOVE:
			rc=rename(file, backupname);
			btype="mv";
			break;
		case BACKUP_LINK:
			rc=link(file, backupname);
			btype="ln";
			break;
		default:
			return -1;
		}
		if(rc) {
			newlog(ERRLOG,
			       "cannot backup: %s %s %s: %s",
			       btype, file, backupname,
			       strerror(errno));
			return 1;
		} else {
			return 0;
		}
	}
	return -1;
}
