/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1999       Matthias Andree
 *  Copyright (C) 2000       Krischan Jodies
 *  Copyright (C) 2000       Dirk Meyer
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
 * backup.c
 *
 * Backup eines Puffers anlegen
 *
 */

/* Changelog: 
 *
 * 11.1.99
 * Logrotation dazugetan, Krischan 
 *
 */

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
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>


#include "utility.h"
#include "calllib.h"
#include "uulog.h"
#include "uudebug.h"
#include "ministat.h"

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

int
backup2(const char *backupdir, const char *file,
	const char *sysname, const char *arcer)
{
	newlog(DEBUGLOG, "backup.c Anzahl Backups: %s",backupnumber);
	if (backupdir && file && sysname) {
		char backupname[FILENAME_MAX];
		char subdir[FILENAME_MAX];
		char backupnamenew[FILENAME_MAX];
		int i;
		int backupnr;
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

		sprintf(subdir,"%s/%s",backupdir,sysname);
		if (backupnumber == NULL) {
			backupnr = 5;
			newlog(ERRLOG,
				"Warnung: Backup-Zahl ist nicht definiert.");
		} else {
			backupnr = atoi(backupnumber);	
			if (backupnr < 1) {
				backupnr = 5;
				newlog(ERRLOG,
			"Warnung: Backup-Zahl ist 0. Nehme stattdessen 5.");
			}
		}
		mkdir (subdir, 0700); /* *** pruefen */
		/* Rotieren */
		for (i = (backupnr-1);i > 0;i--) {
			sprintf(backupname,"%s/%s.%d.%s",
				subdir, sysname, i, arcer );
			sprintf(backupnamenew,"%s/%s.%d.%s",
				subdir, sysname, i+1, arcer );
			newlog(DEBUGLOG,"old: %s",backupname);
			newlog(DEBUGLOG,"new: %s",backupnamenew); 
			/* Wenns die Datei nicht gibt,
			   oder sie eine normale Datei ist, ok */
			if ( lstat(backupnamenew,&st) != ENOENT ) { 
			     if  (!S_ISREG(st.st_mode) ) {
				snprintf(backupname,sizeof(backupname)-1,
					"%s/%s.%ld.%s",
					subdir, sysname,
					(long)time(NULL), arcer );
				newlog(ERRLOG,
		"Backup: Fehler %s ist keine regulaere Datei."
		" Speichere Backup unter %s", backupnamenew, backupname );
				break;
			     }
			}
			rename(backupname,backupnamenew);
		}
		/* Wenn die Schleife durch ist,
		   steht in backupname system.1.arcer, oder
                   backupname hat einen Wert auf den geschrieben werden kann */
		newlog(DEBUGLOG,"rename %s -> %s",file,backupname);
		rename(file,backupname);
		return 1;

	}
	return -1;
}
