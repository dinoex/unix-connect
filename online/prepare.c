/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
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
 *  prepare.c:
 *
 *	Verschiebt die zu versendenden Dateien in das Upload-Directory
 *	und erzeugt die entsprechenden Eintraege in der Warteschlange.
 */

#include "config.h"
#include "zconnect.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#else
#ifndef R_OK
#include <sys/file.h>
#endif
#endif

#include "utility.h"
#include "xprog.h"
#include "locknames.h"

/*
 *  Alle relevanten Informationen stehen in "connection".
 */

extern int ist_testaccount;

void bereitstellen(void)
{
	char buffer[FILENAME_MAX], buffer2[FILENAME_MAX], arc[20];
	char lockname[FILENAME_MAX];
	struct stat info;

	fputs("Bereitstellen unserer Daten\n", deblogfile);
	fflush(deblogfile);

	strncpy(arc, connection.local_arc, sizeof arc);
	arc[sizeof arc -1] = '\0';
	strlwr(arc);
	sprintf(buffer, "%s/upload", connection.tmp_dir);
	connection.upload_dir = dstrdup(buffer);
	mkdir(buffer, 0777);

	sprintf(buffer, "%s/brt.%s.%s", netcalldir, connection.remote_sys, arc);
	if (stat(buffer, &info) == 0) {
		sprintf(buffer2, "%s/brt.%s", connection.upload_dir, arc);
		fprintf(deblogfile, "News: %ld Bytes\n", (long)info.st_size);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, connection.remote_sys);
		if (waitnolock(lockname, 180)) {
			fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
			exit(1);
		}
		if (rename(buffer, buffer2) != 0) {
			fprintf(deblogfile, "rename() fehlgeschlagen: %s\n", strerror(errno));
			exit(1);
		}
		sprintf(buffer, "brt.%s", arc);
		queue_action(connection.upload_dir, buffer, MSG_SEND, BRETT, !ist_testaccount);
	} else {
		fprintf(deblogfile, "Keine Brettnachrichten zu verschicken: %s\n", buffer);
	}

	sprintf(buffer, "%s/prv.%s.%s", netcalldir, connection.remote_sys, arc);
	if (stat(buffer, &info) == 0) {
		sprintf(buffer2, "%s/prv.%s", connection.upload_dir, arc);
		fprintf(deblogfile, "PMs: %ld Bytes\n", (long)info.st_size);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, connection.remote_sys);
		if (waitnolock(lockname, 180)) {
			fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
			exit(1);
		}
		if (rename(buffer, buffer2) != 0) {
			fprintf(deblogfile, "rename(%s, %s) fehlgeschlagen: %s\n", buffer, buffer2, strerror(errno)); /* Fehlermeldungen ueber Dateien bitte generell mit vollen pathnames. Danke.*/
			exit(1);
		}
		sprintf(buffer, "prv.%s", arc);
		queue_action(connection.upload_dir, buffer, MSG_SEND, PM, !ist_testaccount);
	} else {
		fprintf(deblogfile, "Keine PMs zu verschicken: %s\n", buffer);
	}
}

extern char tty[];

/*
 *  Entfernt den uebrigen Schrott, schiebt nicht-uebertragene Dateien
 *  zurueck und liest schliesslich die empfangenen Daten ein.
 */
void aufraeumen(void)
{
	char buffer[FILENAME_MAX], buffer2[FILENAME_MAX], arc[20];
	char lockname[FILENAME_MAX];

	strncpy(arc, connection.local_arc, sizeof arc);
	arc[sizeof arc -1] = '\0';
	strlwr(arc);

	alarm(0);
	signal(SIGHUP, SIG_IGN);

	fputs("Fertig, raeume auf\n", deblogfile);

	/* Fertig, Modem auflegen */
	fclose(stderr);	DMLOG("close stderr");
	fclose(stdin); DMLOG("close stdin");
	restore_linesettings(fileno(stdout)); DMLOG("restoring modem parameters on stdout");
	hangup(fileno(stdout)); DMLOG("hangup stdout");
	fclose(stdout); DMLOG("close stdout");
	fopen("/dev/null", "r");		/* neuer stdin */
	fopen("/dev/null", "w");		/* neuer stdout */
	fopen("/dev/null", "w");		/* neuer stderr */
	lock_device(0, tty);

	chdir(connection.upload_dir);	/* Das sollte jetzt leer sein */
	sprintf(buffer, "brt.%s", arc);
	if (access(buffer, R_OK) == 0) {
		fprintf(deblogfile, "Fehler: Datei %s blieb liegen\n", buffer);
		newlog(ERRLOG, "Fehler: Datei %s blieb liegen", buffer);
		sprintf(buffer2, "%s/brt.%s.%s", netcalldir, connection.remote_sys, arc);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, connection.remote_sys);
		if (waitnolock(lockname, 180)) {
			fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
			exit(1);
		}
		if (access(buffer2, R_OK) != 0) {
			rename(buffer, buffer2);
		} else {
			/*
			 *  Schade, da ist schon wieder ein ARC, wir muessen
			 *  die Daten im Spool-Verzeichnis auspacken.
			 */
			sprintf(buffer2, "%s/%s", connection.upload_dir, buffer);
			chdir(netcalldir);
			chdir(connection.remote_sys);
			call_auspack(connection.local_arc, buffer2);
			remove(buffer2);
		}
	}

	chdir(connection.upload_dir);	/* Das sollte jetzt leer sein */
	sprintf(buffer, "prv.%s", arc);
	if (access(buffer, R_OK) == 0) {
		fprintf(deblogfile, "Fehler: Datei %s blieb liegen\n", buffer);
		newlog(ERRLOG, "Fehler: Datei %s blieb liegen", buffer);
		sprintf(buffer2, "%s/prv.%s.%s", netcalldir, connection.remote_sys, arc);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, connection.remote_sys);
		if (waitnolock(lockname, 180)) {
			fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
			exit(1);
		}
		if (access(buffer2, R_OK) != 0) {
			rename(buffer, buffer2);
		} else {
			/*
			 *  Schade, da ist schon wieder ein ARC, wir muessen
			 *  die Daten im Spool-Verzeichnis auspacken.
			 */
			sprintf(buffer2, "%s/%s", connection.upload_dir, buffer);
			chdir(netcalldir);
			chdir(connection.remote_sys);
			call_auspack(connection.local_arc, buffer2);
			remove(buffer2);
		}
	}

	chdir(connection.tmp_dir);
	rmdir(connection.upload_dir);
	fclose(deblogfile);
	fflush(stdout);
	fflush(stderr);

	/*
	 * Und empfangene Daten (im Hintergrund) einlesen,
	 * das Modem wird sofort wieder freigegeben.
	 */
	if (fork() == 0) {
		char logfname[FILENAME_MAX];

		/* Ich bin child */
		sleep(5);	/* Gib dem 'parent' Zeit sich zu beenden */
		sprintf(logfname, "%s/" XTRACTLOG_FILE, logdir);
		deblogfile = fopen(logfname, "a");
		close(fileno(stderr)); dup(fileno(deblogfile));	/* stderr = deblogfile */
		DMLOG("child forked");
		if (chdir (connection.tmp_dir) != 0) {
			perror(connection.tmp_dir);
			fprintf(deblogfile, "\n\nFATAL: kann nicht nach %s chdir()'en: %s\n", connection.tmp_dir, strerror(errno));
			_exit(15);
		}
		import_all(connection.remote_arc, connection.remote_sys);
		fclose(deblogfile);
		chdir ("/");
		rmdir(connection.tmp_dir);
		_exit(0);
	}
}
