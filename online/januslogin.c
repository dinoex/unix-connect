/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-96  Christopher Creutzig
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
 * JANUS Login fuer UNIX
 *
 *  ACHTUNG: eine sichere Funktion bei der Annahme von JANUS
 *           Anrufen durch ein UNIX System kann derzeit nicht gewaehrleistet
 *           werden!
 */

#include "config.h"
#include "utility.h"
#include "zconnect.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAS_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>

#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "version.h"
#include "track.h"
#include "line.h"
#include "boxstat.h"
#include "ministat.h"
#include "uulog.h"
#include "xprog.h"
#include "lib.h"
#include "locknames.h"

static jmp_buf timeout, nocarrier;
static char *tty = "/dev/tty";

FILE *deblogfile;

#define	WHO	"JANUS"

#define NAK	0x15
#define ACK	0x06

void handle_timeout(int code);
void handle_timeout(int code)
{
	longjmp(timeout, 1);
}

void handle_nocarrier(int code);
void handle_nocarrier(int code)
{
	longjmp(nocarrier, 1);
}

void getsln(char *p);
void getsln(char *p)
{
	int z;

	while (1) {
		z = getchar();
		if (z == '\r') break;
		if (isprint(z))
			*p++ = z;
	}
	*p = '\0';
	fputs("\r\n", stdout);
}

void getln(char *p);
void getln(char *p)
{
	int z;

	while (1) {
		z = getchar();
		if (z == '\r') break;
		if (isprint(z))
			*p++ = z;
		putc(z, stdout);
	}
	*p = '\0';
	fputs("\r\n", stdout);
}

static void
findsysfile(const char *sysdir, char *sysname, char *filename)
{
	DIR *dir;
	struct dirent *this;
	int len;
	char buf[FILENAME_MAX], *p;

	len = strlen(sysname);
	strlwr(sysname);
	dir = opendir(sysdir);
	while ((this = readdir(dir)) != NULL) {
		strcpy(buf, this->d_name);
		p = strchr(buf, '.');
		if (p) *p = '\0';
		strlwr(buf);
		if (stricmp(buf, sysname) == 0) {
			strcpy(sysname, this->d_name);
			break;
		}
	}
	closedir(dir);
	sprintf(filename, "%s/%s", sysdir, sysname);
}

void reject(char *prog, char *sysname, char *passwd);
void reject(char *prog, char *sysname, char *passwd)
{
	fputs("Netzzugriff verweigert.\r\n", stdout);
	fprintf(deblogfile,
		"%s: Fehlversuch (System %s Passwort %s)\n",
		prog, sysname, passwd);
	newlog(INCOMING,
		"%s: Fehlversuch (System %s Passwort %s)",
		prog, sysname, passwd);
	lock_device(0, tty);
	fclose(deblogfile);
	exit(0);
}

/*
 *   Hauptprogramm, Gesamtstruktur
 */
int main(int argc, char **argv)
{
	char sysname[FILENAME_MAX];
	char passwd[80];
	char filename[FILENAME_MAX];
	char lockname[FILENAME_MAX];
	char tmpname[FILENAME_MAX];
	char outname[FILENAME_MAX];
	char *arcer, *arcerin, *transfer, sernr[6];
	int i, j, z, serchk;
	FILE *f;
	header_p hd, p;

	initlog("januslogin");

	/*
	 *  Da dieses Programm Login-Shell ist, muss die Fehlerausgabe
	 *  irgendwo hin dirigiert werden...
	 */
	deblogfile = fopen("/tmp/zlog", "a");
	if (!deblogfile) {
		deblogfile = fopen("/dev/null", "w");
		if (!deblogfile) {
			printf("Sorry - kann Logfile nicht schreiben...\n");
			return 10;
		}
	}

	freopen("/tmp/januslogin-stderr.log", "w", stderr);

	/*
	 *  Konfiguration einlesen.
	 */
	minireadstat();

	/*
	 *  Verschiedene Abbruch-Gruende vorbereiten
	 */
	if (setjmp(timeout)) {
		fputs("\nABBRUCH: Timeout\n", deblogfile);
		fclose(deblogfile);
		newlog(INCOMING, "Abbruch: Timeout");
		return 1;
	}
	if (setjmp(nocarrier)) {
		fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", deblogfile);
		fclose(deblogfile);
		newlog(INCOMING, "Abbruch: No Carrier");
		return 2;
	}
	signal(SIGHUP, handle_nocarrier);
	signal(SIGALRM, handle_timeout);

	/*
	 *  Hier beginnt der eigentliche Datenaustausch
	 */
	set_rawmode(fileno(stdin));
	set_rawmode(fileno(stdout));

	tty = ttyname(0);
	lock_device(1, tty);

	alarm(45);
ask_sysname:
	sleep(1); fflush(stdin);
	fputs("Systemname: ", stdout); fflush(stdout);
	getln(sysname);
	if (!sysname[0]) goto ask_sysname;	/* z.B. bei Ctrl-X */
	strlwr(sysname);
	if (strcmp(sysname, "zerberus") == 0) goto ask_sysname;
	if (strcmp(sysname, "janus") == 0) goto ask_sysname; /* Wenn schon, denn schon */
ask_passwort:
	sleep(1); fflush(stdin);
	fputs("Passwort: ", stdout); fflush(stdout);
	getsln(passwd);
	if (stricmp(passwd, "zerberus") == 0) goto ask_sysname;
	if (stricmp(passwd, "janus") == 0) goto ask_sysname; /* Hier auch nochmal */
	if (stricmp(sysname, passwd) == 0) goto ask_passwort;
	alarm(0);

	findsysfile(systemedir, sysname, filename);
	f = fopen(filename, "rb");
	if (!f) {
		fprintf(deblogfile, "Unbekanntes System: %s\n", sysname);
		reject(argv[0], sysname, passwd);
	}
	hd = rd_para(f);
	fclose(f);
	p = find(HD_SYS, hd);
	if (p) {
		char buf[FILENAME_MAX], *c;
		strcpy(buf, sysname);
		c = strchr(buf, '.');
		if (c) *c = '\0';
		if (stricmp(buf, p->text) != 0) {
			fprintf(deblogfile, "Unbekanntes System: %s\n", sysname);
			reject(argv[0], sysname, passwd);
		}
	} else {
		fprintf(deblogfile, "Illegale Systemdatei: Kein "HN_SYS": Header: %s\n", filename);
		reject(argv[0], sysname, passwd);
	}

	p = find(HD_PASSWD, hd);
	if (!p || stricmp(p->text, passwd) != 0) {
		fputs("Passwort falsch:\n", deblogfile);
		reject(argv[0], sysname, passwd);
	}

	p = find(HD_ARCEROUT, hd);
	if (!p) {
		fputs("Kein ARCer (ausgehend) definiert!\n", deblogfile);
		reject(argv[0], sysname, passwd);
	}
	arcer = p->text;
	strlwr(arcer);

	p = find(HD_ARCERIN, hd);
	if (!p) {
		fputs("Kein ARCer (eingehend) definiert!\n", deblogfile);
		reject(argv[0], sysname, passwd);
	}
	arcerin = p->text;
	strlwr(arcerin);

	p = find(HD_PROTO, hd);
	if (!p) {
		fputs("Kein Uebertragungsprotokoll definiert!\n", deblogfile);
		reject(argv[0], sysname, passwd);
	}
	transfer = p->text;
	strlwr(transfer);

	fputs("Running ARC....\r\n"
	     "Running ARC....\r\n"
	     "Running ARC....\r\n"
	     "Running ARC....\r\n", stdout);
	fprintf(deblogfile,
		"%s: System %s erfolgreich eingelogt [%s]\n",
		argv[0], sysname, tty);
	newlog(INCOMING,
		"%s: System %s erfolgreich eingelogt [%s]",
		argv[0], sysname, tty);

	sprintf(tmpname, "%s/%s.%d.dir", netcalldir, sysname, getpid());
	mkdir(tmpname, 0777);

	if (setjmp(timeout)) {
		fputs("\nABBRUCH: Timeout\n", deblogfile);
		newlog(INCOMING, "Abbruch: Timeout");
		fclose(deblogfile);
		return 1;
	}
	if (setjmp(nocarrier)) {
		fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", deblogfile);
		newlog(INCOMING, "Abbruch: No Carrier");
		fclose(deblogfile);
		return 2;
	}

	chdir(tmpname);
	sprintf(outname, "%s/called.%s", tmpname, arcer);
	sprintf(filename, "%s/%s.%s", netcalldir, sysname, arcer);
	sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, sysname);
	if (access(filename, R_OK) != 0) {
		f = fopen(outname, "wb");
		if (!f) {
			fprintf(deblogfile, "Kann Netcall %s nicht erzeugen\n", outname);
			fclose(deblogfile);
			return 1;
		}
		fputs("\r\n", f);
		fclose(f);
	} else {
		if (waitnolock(lockname, 180)) {
			fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
			fclose(deblogfile);
			newlog(INCOMING, "Prearc LOCK: %s", lockname);
			return 1;
		}
		if(rename(filename, outname)) {
			fprintf(deblogfile, "Umbenennen: %s -> %s fehlgeschlagen\n", filename, outname);
			fclose(deblogfile);
			return 1;
		}
	}
	sprintf(filename, "caller.%s", arcer);
	sprintf(outname, "called.%s", arcer);

	if (setjmp(timeout)) {
		fputs("\nABBRUCH: Timeout\n", deblogfile);
		newlog(INCOMING, "Abbruch: Timeout");
		goto cleanup;
	}
	if (setjmp(nocarrier)) {
		fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", deblogfile);
		newlog(INCOMING, "Abbruch: No Carrier");
		goto cleanup;
	}

	/*
	 *  Man glaubt es kaum, aber einige Programme schaffen es nicht,
	 *  ein Packzeit von 0 Sekunden der Gegenseite zu verstehen...
	 */
	sleep(3);

	alarm(30);
	for (i=1; i<7; i++) {
		putchar(NAK); fflush(stdout);
		z = 0;
		for (j=0; j<4; j++)
			sernr[j] = toupper(getchar());
		serchk = getchar();
/* Die Pruefsumme der Seriennummer zu pruefen, ist eigentlich unnoetig. ccr */
/*		if ((sernr[0]+sernr[1]+sernr[2]+sernr[3]) == serchk & 0x0ff)*/ {
			alarm(0);
			putchar(ACK); fflush(stdout);
			sernr[4] = '\0';
			newlog(INCOMING, WHO " Seriennummer: %s\n", sernr);
			if (recvfile(transfer, filename)) {
				fprintf(deblogfile,
					"Empfang der Daten fehlgeschlagen!\n");
				newlog(INCOMING,
					WHO "Empfang der Datei %s "
					"fehlgeschlageni mit %s",
					filename, transfer );
				break;
			}
			if (sendfile(transfer, outname)) {
				fprintf(deblogfile,
					"Versand der Daten fehlgeschlagen\n");
				newlog(INCOMING,
					WHO "Versand der Datei %s "
					"fehlgeschlagen mit %s",
					outname, transfer );
				break;
			}

			/* Fertig, Modem auflegen */
			signal(SIGHUP, SIG_IGN);
			fclose(stderr);
			fclose(stdin);
			hangup(fileno(stdout));
			fclose(stdout);
			fclose(deblogfile);
			lock_device(0, tty);

			/* Netcall war erfolgreich, also Daten loeschen */
			newlog(INCOMING, WHO " Netcall erfolgreich");
			remove(outname);

			/*
			 * Und empfangene Daten (im Hintergrund) einlesen,
			 * das Modem wird sofort wieder freigegeben.
			 */
			if (fork() == 0) {
				/* Ich bin child */
				import_all(arcerin, sysname, 0);
				chdir ("/");
				rmdir(tmpname);
			}

			return 0;
		}
	}

cleanup:
	alarm(0);
	fprintf(deblogfile, "Netcall fehlgeschlagen\n");
	newlog(INCOMING, "Netcall fehlgeschlagen");
	lock_device(0, tty);

	/* Jetzt muessen die Daten zurueck aus dem temporaeren Verzeichnis
	 * ins Haupt-Netcall-Verzeichnis
	 */
	sprintf(filename, "%s/%s.%s", netcalldir, sysname, arcer);
	sprintf(outname, "%s/called.%s", tmpname, arcer);
	if (waitnolock(lockname, 180)) {
		fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
		newlog(INCOMING, "Prearc LOCK: %s", lockname);
		fclose(deblogfile);
		return 1;
	}
	if (access(filename, R_OK) != 0) {
		/*
		 * Der einfache Fall: es ist noch nichts neues da,
		 * wir koennen die ARC Datei an die urspruengliche
		 * Stelle zurueckschieben.
		 */
		rename(outname, filename);
	} else {
		/*
		 * Uups, da hat schon jemand wieder gearct - dann packen
		 * wir die Daten einfach wieder im Sammel-Verzeichnis aus.
		 * Sie werden dann beim naechsten PreARC Lauf miterfasst.
		 */
		chdir(netcalldir);
		chdir(sysname);
		call_auspack(arcer, outname);
		remove(outname);
	}
	chdir("/");
	rmdir(tmpname);

	fclose(deblogfile);

	return 0;
}
