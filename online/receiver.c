/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 *  receiver.c
 *
 *    Die Top-Level Routinen sowie die Integration in das Gesamtsystem
 */

#include "config.h"
#include "zconnect.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif


/*
 * Welches Zeilenende soll wr_para verwenden?
 * Da hier nur interne Konfigurationsdateien geschrieben werden, benutzen
 * wir das UNIX-interne Zeilenende.
 */
const char *hd_crlf = "\n";
int logname = INCOMING;

char tty[FILENAME_MAX] = "/dev/tty";

jmp_buf timeout, nocarrier;

FILE *deblogfile;

static void
handle_timeout(int code)
{
	longjmp(timeout, 1);
}

static void
handle_nocarrier(int code)
{
	longjmp(nocarrier, 1);
}

#ifdef TCP_SUPPORT
int tcp = 0;
#endif

/*
 *   Hauptprogramm, Gesamtstruktur
 */
int main(int argc, char **argv)
{
	volatile int file_phase;

	initlog("receiver");

	freopen("/tmp/receiver-stderr.log", "w", stderr);

#ifdef TCP_SUPPORT
	if (argc == 2 && strcmp(argv[1], "-l") == 0) {
		banner_login();
		tcp = 1;
	}
#endif

	minireadstat();
	/*
	 *  Da dieses Programm Login-Shell ist, muss die Fehlerausgabe
	 *  irgendwo hin dirigiert werden...
	 */
	deblogfile = fopen("/tmp/clog", "w");
	if (!deblogfile) {
		deblogfile = fopen("/dev/null", "w");
		if (!deblogfile) {
			printf("Sorry - kann Logfile nicht schreiben...\n");
			return 10;
		}
	}

	/*
	 *  Verschiedene Abbruch-Gruende vorbereiten
	 */
	if (setjmp(timeout)) {
		fputs("\nABBRUCH: Timeout\n", deblogfile);
		if (file_phase) {
			aufraeumen();
		} else {
			fclose(deblogfile);
			lock_device(0, tty);
		}
		return 1;
	}
	if (setjmp(nocarrier)) {
		if (!auflegen)
			fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", deblogfile);
		if (file_phase) {
			aufraeumen();
		} else {
			fclose(deblogfile);
			lock_device(0, tty);
		}
		return 2;
	}
	signal(SIGHUP, handle_nocarrier);
	signal(SIGALRM, handle_timeout);

	strcpy(tty, ttyname(0));
	lock_device(1, tty);

	/*
	 *  Hier beginnt der eigentliche ZCONNECT Datenaustausch
	 */
	file_phase = 0;
	prologue();		/* Passwort fragen, BEGIN senden .... */
	system_dialog();	/* Systeminfos austauschen, pruefen ... */
	if (!auflegen && !auflegen_empfangen)
		bereitstellen();
	file_phase = 1;
	while (!auflegen)	/* Datenaustausch, bis nichts mehr zu */
		datei_dialog();	/* tauschen ist */

	/*
	 *  Daten sind uebertragen, jetzt muessen sie noch ausgepackt und
	 *  eingelesen werden...
	 */

	 aufraeumen();

	 return 0;
}
