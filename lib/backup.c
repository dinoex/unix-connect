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
#include "idir.h"
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "istring.h"
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
		char systemdir[FILENAME_MAX];
		char backupnamenew[FILENAME_MAX];
		char olddir[FILENAME_MAX];
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

		snprintf(systemdir,sizeof(systemdir)-1,"%s/%s",
				backupdir,sysname);
		if (backupnumber == NULL) {
			backupnr = 5;
			newlog(ERRLOG,
				"Warnung: Backup-Zahl ist nicht definiert."
				" Nehme einfach 5.");
		} else {
			backupnr = atoi(backupnumber);
			if (backupnr < 1) {
				backupnr = 5;
				newlog(ERRLOG,
					"Warnung: Backup-Zahl ist 0."
					" Nehme stattdessen 5.");
			}
		}
		if ( (mkdir (systemdir, 0700) < 0) && (errno != EEXIST) ) {
			newlog(ERRLOG,
				"Kann Verzeichnis %s nicht anlegen (%s)."
				" Backup nicht erzeugt!",
				systemdir,strerror(errno));
			return 1;
		}
		if ( getcwd(olddir,FILENAME_MAX-1) != NULL ) {
		      newlog(ERRLOG,
				"Cannot get actual directoryr %s:",
				strerror(errno));
			return 1;
		}
		/* Ins Systemverzeichnis wechseln,
		   damit ENOENT bei lstat bedeutet,
		   dass die Datei nicht da ist
		   (und nicht evtl. dass im Pfad ein Eintrag ungueltig ist)
		 */
		newlog(DEBUGLOG,"Backup: chdir(%s)",systemdir);
		if (chdir(systemdir) < 0 ) {
		      newlog(ERRLOG,"Cannot chdir to %s (%s)",
				systemdir,strerror(errno));
		      return 1;
		}
		/* Rotieren */
		for (i = (backupnr-1);i > 0;i--) {
			sprintf(backupname,   "%s.%d.%s",sysname,i,arcer);
			sprintf(backupnamenew,"%s.%d.%s",sysname,i+1,arcer);
			/* Wenns die Datei nicht gibt,
			   oder sie eine normale Datei ist, ok */
			if (! lstat(backupnamenew,&st)) {
				if (errno != ENOENT) {
					newlog(ERRLOG,
		"Backup: Fehler beim Rotieren und lstat auf %s/%s (%s)",
						systemdir,backupnamenew,
						strerror(errno));
					chdir(olddir);
					return 1;
				}
			} else {
				if (! S_ISREG(st.st_mode) ){
					newlog(ERRLOG,
				"Backup: Fehler %s ist keine regulaere Datei.",
						backupnamenew);
					chdir(olddir);
					return 1;
				}
			}
			newlog(DEBUGLOG,"old: %s",backupname);
			newlog(DEBUGLOG,"new: %s",backupnamenew);
			if ( (rename(backupname,backupnamenew) < 0)
			&& (errno != ENOENT) ) {
				newlog(ERRLOG,
				"Backup: rename(%s,%s) schlug fehl (%s)",
					backupname,backupnamenew,
					strerror(errno));
				chdir(olddir);
				return 1;
			}
			rename(backupname,backupnamenew);
		}
		chdir(olddir);

		/* Wenn die Schleife durch ist,
		   steht in backupname system.1.arcer */
		snprintf(backupnamenew,sizeof(backupnamenew)-1,"%s/%s",
			systemdir,backupname);
		newlog(DEBUGLOG,"rename %s -> %s",file,backupnamenew);
		if (rename(file,backupnamenew) < 0) {
			newlog(ERRLOG,
				"Backup: rename(%s,%s) schlug fehl (%s)",
				file,backupnamenew,strerror(errno));
		}
		return 0;

	}
	return -1;
}

