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
 *  call.c
 *
 *  Temporaeres Hilfsprogramm zum Anwaehlen eines anderen Systems.
 *  Dieser Teil (Hauptprogramm) wird spaeter komplett ersetzt.
 *
 *  ACHTUNG: damit dies funktioniert, muss der UUCP Locking-Mechanismus
 *	     korrekt installiert sein. Sieh dazu ../include/policy.h
 *
 * -----------------------------------------------------------------------
 *
 *   Letzte Aenderung: martin@bi-link.owl.de, Mon Nov 22 22:55:39 1993
 */

#include "config.h"
#include "zconnect.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAS_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <fcntl.h>
#ifdef NO_UNISTD_H
#ifndef R_OK
#include <sys/file.h>
#endif
#else
#include <unistd.h>
#endif
#include "lib.h"
#include "track.h"
#include "hayes.h"
#include "xprog.h"
#include "locknames.h"
#ifdef BSD
#include <sys/ioctl.h>	/* setup our own controlling tty */
#endif

/*
 * Falls das angerufene System einen FIDO-Mailer installiert hat,
 * brechen wir dessen EMSI-Sequenzen ab. Ansonsten würden wir bis
 * zum Timeout (10 - 20 s) warten müssen.
 */
#define EMSI_REQ		"**EMSI_REQA77E"	/* vom Fido-Mailer */
#define EMSI_CLI		"**EMSI_CLIF8AC\r" 	/* Abbruchsequenz */

/*
 * Welches Zeilenende soll wr_para verwenden?
 * Da hier nur interne Konfigurationsdateien geschrieben werden, benutzen
 * wir das UNIX-interne Zeilenende.
 */
char *hd_crlf = "\n";
#ifdef LOGSYSLOG
int logname = OUTGOING;
#else
char *logname = OUTGOING;
#endif

#define NAK	0x15
#define ACK	0x06

jmp_buf timeout, nocarrier;

FILE *deblogfile;
static time_t online_start;
static files = 0;

char int_prefix[20];	/* international prefix, z.B. 49 fuer Deutschland */
char ovst[40];		/* Ortsnetz, z.B. 521 fuer Bielefeld */
void setup_dial_info(const char *intnl,
	char *int_prefix, char *ovst, char *telno);

#ifndef O_RDWR
#define O_RDWR	2
#endif

void handle_timeout(int code)
{
	longjmp(timeout, 1);
}

void handle_nocarrier(int code)
{
	longjmp(nocarrier, 1);
}


void usage(void)
{
	fputs (	"Aufruf: zcall (System) (Device) (Speed) [Anzahl der Versuche]\n"
		"um ein ZCONNECT- oder JANUS-System anzurufen\n", stderr);
	exit(1);
}

int anruf (char *intntl, header_p sys, header_p ich, int ttyf);

char tty[FILENAME_MAX];
int modem;

void cleanup() {
  if(modem>-1)
    restore_linesettings(modem);
}

int main(int argc, char **argv)
{
	char *sysname, *speed;
	char sysfile[FILENAME_MAX], deblogname[FILENAME_MAX];
	FILE *f;
	header_p sys, ich, p;
	time_t now;
	int i;
	static volatile int maxtry;

	modem=-1;
	atexit(cleanup);

	if (argc < 4 || argc > 5) usage();
	sysname = argv[1];
	if (!strchr(argv[2], '/'))
		sprintf(tty, "/dev/%s", argv[2]);
	else
		strcpy(tty, argv[2]);
	strlwr(sysname);
	speed = argv[3];
	if (argc > 4)
		maxtry = atoi(argv[4]);
	else
		maxtry = 1;
	minireadstat();
#ifdef LOGSYSLOG
	sprintf(deblogname, "%s/" DEBUGLOG_NAME, logdir);
#else
	sprintf(deblogname, "%s/" DEBUGLOG, logdir);
#endif
	if(debuglevel>0) {
		deblogfile = fopen(deblogname, "w");
		if (!deblogfile) {
			printf("Ich kann das Logfile nicht öffnen. Probiere /dev/null...\n");
			deblogfile = fopen("/dev/null", "w");
			if (!deblogfile) {
				printf("Hmm... - /dev/null nicht schreibbar??\n");
				return 10;
			}
		}
	} else {
		deblogfile = fopen("/dev/null", "w");
		if (!deblogfile) {
			printf("Arghl! - kann /dev/null nicht zum Schreiben oeffnen!\n\n");
			return 10;
		}
	}
	sprintf(sysfile, "%s/%s", systemedir, sysname);
	f = fopen(sysfile, "r");
	if (!f) {
		perror(sysfile);
		logfile(ERRLOG, __FILE__, sysfile, "-", "%s\n", strerror(errno));
		return 1;
	}
	sys = rd_para(f);
	fclose(f);

	ich = get_myself();

	for (i=maxtry; i; i--) {
	   if (lock_device(1, tty)) break;
	   fputs(" ... unser Modem ist belegt\n", stderr);
	   sleep(60);
	}
	if (!i) {
		fprintf(deblogfile, "Cannot lock device: %s\n", tty);
		fprintf(stderr, "Kann %s nicht reservieren...\n", tty);
		return 9;
	}

#ifdef LEAVE_CTRL_TTY
	/*
	 * Bisheriges Controlling-TTY verlassen, damit "modem" das neue
	 * wird.
	 */
#ifdef BSD
	setpgrp(0, getpid());	/* set process group id to process id */
#ifdef SIGTTOU
	signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif
#else	/* !BSD */
#if !defined(USE_SETSID) && !defined(USE_SETPGRP)
#error Controlling TTY kann nicht verlassen werden: definieren Sie eine Methode dazu
#endif /* !BSD und keine SysV-Methode definiert */

#ifdef USE_SETSID
#error	Dies funktioniert nicht!
	setsid();
#endif /* USE_SETSID */
#ifdef USE_SETPGRP
	setpgrp();
#endif /* USE_SETPGRP */
#endif /* !BSD */
#endif /* LEAVE_CTRL_TTY */

	modem = open(tty,
#if !defined(linux) && !defined(__NetBSD__)
			O_RDWR | O_NDELAY
#else
			O_RDWR
#endif
		); DMLOG("open modem");
	if (modem < 0) {
		perror(tty);
		fprintf(deblogfile, "Can not access device %s: %s\n", tty, strerror(errno));
		return 10;
	}
	save_linesettings(modem); DMLOG("saving modem parameters");
	set_rawmode(modem); DMLOG("set modem to rawmode");
	set_local(modem, 1);
	set_speed(modem, speed); DMLOG("set modem speed");
#ifdef TIOCSCTTY
	ioctl(modem, TIOCSCTTY, NULL);
#endif

	files = 0;
	online_start = 0;
	fprintf(stderr, "Netcall bei %s [%s]\n", sysname, tty);
	
	close(fileno(stdin));
	close(fileno(stdout));
	dup(modem); dup(modem); DMLOG("dup modem 2x to stdin and stdout");

	if (setjmp(timeout)) {
		fputs("\nABBRUCH: Timeout\n", stderr);
		fputs("\nABBRUCH: Timeout\n", deblogfile);
		lock_device(0, tty);
		if (online_start) {
			time(&now);
			fprintf(stderr, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
			fprintf(deblogfile, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
			logfile(logname, "-", "-", "-", "%ld Sekunden online", (long)(now-online_start));
		}
		if (files) aufraeumen();

		return 11;
	}
	if (setjmp(nocarrier)) {
		if (!auflegen) {
			fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", stderr);
			fputs("\nABBRUCH: Gegenstelle hat aufgelegt!\n", deblogfile);
		}
		lock_device(0, tty);
		if (online_start) {
			time(&now);
			fprintf(stderr, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
			fprintf(deblogfile, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
			logfile(logname, "-", "-", "-", "%ld Sekunden online", (long)(now-online_start));
		}
		if (files) aufraeumen();

		return auflegen ? 0 : 12;
	}
	signal(SIGHUP, handle_nocarrier);
	signal(SIGALRM, handle_timeout);
	setup_dial_info(ortsnetz, int_prefix, ovst, NULL);

	while(maxtry) {
		p = find(HD_TEL, sys);
		if (!p) {
			fprintf(stderr, "Keine Telefonnummer fuer %s gefunden!\n", sysname);
			logfile(ERRLOG, __FILE__, sysfile, "-", "Keine Telefonnummer fuer %s gefunden!\n", sysname);
			return 2;
		}
	
		while (p && maxtry) {
			DMLOG("place call on modem");
			if (anruf(p->text, sys, ich, modem)) {
				maxtry = 0; break; 
			}
			p = p->other;
			maxtry--;
		}
	}

	if (online_start) {
		time(&now);
		fprintf(stderr, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
	} else
		fprintf(stderr, "Keine Verbindung hergestellt.\n");

	return 0;
}

/*
 *  Fuer "verfahren" :
 */
#define JANUS		0
#define ZCONNECT	1

int login(int modem, int verfahren, const char *myname, const char *passwd)
{
        int     t_ogin, t_ame, t_wort, t_word, t_begin, found,
		t_esc, t_sysname, t_arc, t_verweigert,
		err;
	char	z, pw[40];
	static	char *logstr[] = { "JANUS\r", "zconnect\r" };

	if (verfahren == ZCONNECT)
		strcpy(pw, "0zconnec\r");
	else
		sprintf(pw, "%s\r", passwd);

	free_all_tracks();       
        t_esc = init_track(EMSI_REQ);
	t_ogin = init_track("OGIN");
	if (verfahren == ZCONNECT)
		t_ame = init_track("AME");
	else
		t_ame = init_track("USERNAME");
	t_wort = init_track("WORT");
	t_word = init_track("WORD");
	t_sysname = -1;
	t_begin = -1;
	t_arc = -1;
	t_verweigert = -1;

        alarm(2*60);
	while (1)
	{
		do { } while (read(modem, &z, 1) != 1);
		found = track_char(toupper(z));
		if (!isascii(z) || !iscntrl(z)) {
			putc(z, stderr);
			fflush(stderr);
		} else if (z == '\r') 
			putc('\n', stderr);
		if (found == -1) continue;
		if (found == t_esc) {
                        write(modem, EMSI_CLI, sizeof(EMSI_CLI));
                } else if (found == t_ogin || found == t_ame) {
                	sleep(2);	/* fflush auf der Gegenseite? */
               		set_local(modem, 0);
			write(modem, logstr[verfahren], strlen(logstr[verfahren]));
			if (verfahren < ZCONNECT && t_sysname == -1) 
				t_sysname = init_track("SYSTEMNAME:");
                } else if (found == t_word || found == t_wort) {
			write(modem, pw, strlen(pw));
			if (verfahren == ZCONNECT && t_begin == -1) {
				t_begin = init_track("BEGIN");
			}else {
				t_arc = init_track("ARC");
				t_verweigert = init_track("VERWEIGER");
			}
		} else if (found == t_sysname) {
			write(modem, myname, strlen(myname));
			write(modem, "\r", 1);
		} else if (found == t_begin) {
			err = 0;
			break;
		} else if (found == t_arc) {
			err = 0;
			break;
		} else if (found == t_verweigert) {
			err = 1;
			break;
		}
	}
        free_all_tracks();
        alarm(0);
        
	return err;
}

/*
 * Zerteile eine internationale Telefonnummer (oder den Anfang davon)
 * in die drei Bestandteile.
 * Parameter:
 *	intnl		die Telefonnummer, z.B. +49-521-68000
 *	int_prefix	die internationale Vorwahl, z.B. 49
 *	ovst		die Vorwahl ohne 0, z.B. 521
 *	telno		die eigentliche Rufnummer
 */
void setup_dial_info(const char *intnl,
	char *int_prefix, char *ovst, char *telno)
{
	char *q, *z;

	if (int_prefix) *int_prefix = '\0';
	if (ovst) *ovst = '\0';
	if (telno) *telno = '\0';

	if (!intnl) return;

	if (!int_prefix) return;
	q=(char *)intnl; z=int_prefix;
	while (*q && (*q == '-' || *q == '+'))
		q++;
	while (*q && isdigit(*q))
		*z++ = *q++;
	*z = '\0';

	if (!ovst) return;
	z=ovst;
	while (*q && (*q == '-' || *q == '+'))
		q++;
	while (*q && isdigit(*q))
		*z++ = *q++;
	*z = '\0';

	if (!telno) return;
	while (*q && (*q == '-' || *q == '+'))
		q++;
	strcpy(telno, q);
}

int anruf (char *intntl, header_p sys, header_p ich, int modem)
{
	time_t now;
	char dialstr[60], phone[60], telno[60], vorw[60], country[60];
	char lockname[FILENAME_MAX];
	header_p p;
	char *name, *pw, **v;
	static char *verf[] = { "JANUS", "ZCONNECT", NULL };
	int i, err;
	static int dial_cnt = 1;

	while (*intntl && !isspace(*intntl))
		intntl++;
	while (*intntl && isspace(*intntl))
		intntl++;
	setup_dial_info(intntl, country, vorw, telno);
	if (strcmp(country, int_prefix) == 0) {
		if (strcmp(vorw, ovst) == 0) {
			strcpy(phone, telno);		/* local call */
		} else {
			strcpy(phone, fernwahl);	/* mit Vorwahl */
			strcat(phone, vorw);
			strcat(phone, telno);
		}
	} else {
		strcpy(phone, international ? international:"00");	/* internationaler Anruf */
		strcat(phone, country);
		strcat(phone, vorw);
		strcat(phone, telno);
	}

	p = find(HD_MODEM_DIAL, config);
	if (!p)
		sprintf(dialstr, "AT DP %s", phone);
	else
		sprintf(dialstr, p->text, phone);

	name = NULL;
	p = find(HD_SYS, ich);
	if (p) name = p->text;
	pw = NULL;
	p = find(HD_PASSWD, sys);
	if (p) pw = p->text;
	p = find(HD_X_CALL, sys);
	if (!p) {
		fprintf(stderr, "Welches Netcall-Verfahren????\n");
		exit(20);
	}
	for (i = 0, v = verf; *v; i++, v++)
		if (stricmp(*v, p->text) == 0)
			break;
	if (!*v) return 1;

	fprintf(stderr, "%3d. Anwahlversuch: %-14s ", dial_cnt++, phone);
	fflush(stderr);

	if (redial(dialstr, modem, 1)) return 0;

	time(&online_start);
	
	if (i < ZCONNECT) {
		if (name) dfree(name);
		name = strdup(boxstat.boxname);
		if (!name) name = "???";
		else strupr(name);
		strupr(pw);
	}
	err = login(modem, i, name, pw);

	if (err) return 0;

	if (i < ZCONNECT) {
		char filename[FILENAME_MAX];
		char tmpname[FILENAME_MAX];
		char outname[FILENAME_MAX];
		char sysname[FILENAME_MAX];
		char *arcer, *arcerin, *transfer, *domain;
		header_p p, d;
		char c;
		struct stat st;
		
		fprintf(stderr, "....\n");
		alarm(10*60);
		do {
			do { } while (read(modem, &c, 1) != 1);
		} while (c != NAK);
		alarm(0);
		fprintf(stderr, "Gegenseite hat gepackt\n");
		alarm(15);
		do {
			write(modem, "UNIXD", 5);
			do { } while (read(modem, &c, 1) != 1);
 		} while (c != ACK);
 		alarm(0);
 		fprintf(stderr, "Seriennummern OK\n");

		p = find(HD_SYS, sys);
		if (!p) {
			fprintf(stderr, "Illegale Systemdatei: Kein " HN_SYS ": Header oder falscher Name: %s\n", filename);
			logfile(ERRLOG, __FILE__, filename, "-", "Kein " HN_SYS ": Header oder falscher Name\n");
			return 1;
		}
		d = find(HD_DOMAIN, sys);
		if (!d) {
			fprintf(stderr, "Illegale Systemdatei: Kein " HN_DOMAIN ": Header: %s\n", filename);
			logfile(ERRLOG, __FILE__, filename, "-", "Kein "HN_DOMAIN": Header oder falscher Name\n");
			return 1;
		}

		for (domain = strtok(d->text, " ;,:"); domain; domain = strtok(NULL, " ;,:")) {
			sprintf(sysname, "%s.%s", p->text, domain);
			strlwr(sysname);
			sprintf(tmpname, "%s/%s", netcalldir, sysname);
			if (access(tmpname, R_OK|X_OK) == 0)
				break;
		}
		
		p = find(HD_ARCEROUT, sys);
		if (!p) {
			fputs("Kein ARCer (ausgehend) definiert!\n", stderr);
			logfile(ERRLOG, __FILE__, filename, "-", "Kein ausgehender Packer definiert\n");
			return 1;
		}
		arcer = p->text;
		strlwr(arcer);

		p = find(HD_ARCERIN, sys);
		if (!p) {
			fputs("Kein ARCer (eingehend) definiert!\n", stderr);
			logfile(ERRLOG, __FILE__, filename, "-", "Kein eingehender Packer definiert\n");
			return 1;
		}
		arcerin = p->text;
		strlwr(arcerin);

		p = find(HD_PROTO, sys);
		if (!p) {
			fputs("Kein Uebertragungsprotokoll definiert!\n", stderr);
			logfile(ERRLOG, __FILE__, filename, "-", "Kein Uebertragungsprotokoll definiert\n");
			return 1;
		}
		transfer = p->text;
		strlwr(transfer);

		sprintf(tmpname, "%s/%s.%d.dir", netcalldir, sysname, getpid());
		mkdir(tmpname, 0777);

		chdir(tmpname);
		sprintf(outname, "%s/caller.%s", tmpname, arcer);
		sprintf(filename, "%s/%s.%s", netcalldir, sysname, arcer);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, sysname);

		if (access(filename, R_OK) != 0) {
			FILE *f;
			
			f = fopen(outname, "wb");
			if (!f) {
				fprintf(stderr, "Kann Netcall %s nicht erzeugen\n", outname);
				logfile(ERRLOG, __FILE__, outname, "-", "Kann Netcall nicht erzeugen: %s\n", strerror(errno));
				fclose(deblogfile);
				return 1;
			}
			fputs("\r\n", f);
			fclose(f);
		} else {
			if (waitnolock(lockname, 180)) {
				logfile(OUTGOING, "-", sysname, "-", "Prearc LOCK: %s\n", lockname);
				fprintf(deblogfile, "Prearc LOCK: %s\n", lockname);
				fclose(deblogfile);
				return 1;
			}
			if(rename(filename, outname)) {
				fprintf(stderr, "Umbenennen: %s -> %s fehlgeschlagen\n", filename, outname);
				logfile(ERRLOG, __FILE__, filename, outname, "Umbenennen fehlgeschlagen: %s\n", strerror(errno));
				fclose(deblogfile);
				return 1;
			}
		}
		sprintf(filename, "called.%s", arcer);
		sprintf(outname, "caller.%s", arcer);

		st.st_size = 0;
		stat(outname, &st);
		fprintf(deblogfile, "Sende %s (%ld Bytes) per %s\n", outname, (long)st.st_size, transfer);
		fprintf(stderr, "Sende %s (%ld Bytes) per %s\n", outname, (long)st.st_size, transfer);
		logfile(logname, transfer, "VERSAND", "-", "%ld Bytes\n", (long)st.st_size);
		/* old_stderr = dup(fileno(stderr)); close(fileno(stderr)); dup(modem); DMLOG("dup modem to stderr"); */
		if (sendfile(transfer, outname)) {
			/* close(fileno(stderr)); dup(old_stderr); close(old_stderr); DMLOG("undup stderr"); */
			fprintf(stderr, "Versand der Daten fehlgeschlagen\n");
			logfile(logname, transfer, "VERSAND", "-", "fehlgeschlagen\n");
			fprintf(deblogfile, "Versand der Daten fehlgeschlagen\n");
			return 1;
		}
		/* close(fileno(stderr)); dup(old_stderr); close(old_stderr); DMLOG("undup stderr"); */

		fprintf(deblogfile, "Empfange per %s\n", transfer);
		fprintf(stderr, "Empfange %s\n", transfer);
		logfile(logname, transfer, "EMPFANG", "-", "\n");
		/* old_stderr = dup(fileno(stderr)); close(fileno(stderr)); dup(modem); DMLOG("dup modem to stderr"); */
		if (recvfile(transfer, filename)) {
			/* close(fileno(stderr)); dup(old_stderr); close(old_stderr); DMLOG("undup stderr"); */
			logfile(logname, transfer, "EMPFANG", "-", "fehlgeschlagen\n");
			fprintf(stderr, "Empfang der Daten fehlgeschlagen!\n");
			fprintf(deblogfile, "Empfang der Daten fehlgeschlagen!\n");
			return 1;
		}
		/* close(fileno(stderr)); dup(old_stderr); close(old_stderr); DMLOG("undup stderr"); */
		st.st_size = 0;
		stat(filename, &st);
		logfile(logname, "-", "-", "-", "%ld Bytes empfangen\n", (long)st.st_size);
		fprintf(stderr, "%ld Bytes empfangen\n", (long)st.st_size);
		fprintf(deblogfile, "%ld Bytes empfangen\n", (long)st.st_size);

		time(&now);
		logfile(logname, "Auflegen", "-", "-", "Anrufdauer ca. %ld Sekunden online\n", (long)(now-online_start));
		fprintf(stderr, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
		fprintf(deblogfile, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));

		/* Fertig, Modem auflegen */
		signal(SIGHUP, SIG_IGN);
		fclose(stdin);
		fclose(stdout);
		hangup(modem); DMLOG("hangup modem");
		restore_linesettings(modem); DMLOG("restoring modem parameters");
		close(modem); modem=-1; DMLOG("close modem");
		fopen("/dev/null", "r");	/* neuer stdin */
		dup2(fileno(stderr),fileno(stdout)); /* stderr wird in stdout kopiert */

		/* Netcall war erfolgreich, also Daten loeschen */
		remove(outname);
		fclose(deblogfile);

		/*
		 * Und empfangene Daten (im Hintergrund) einlesen,
		 * das Modem wird sofort wieder freigegeben.
		 */
		if (fork() == 0) {
			/* Ich bin child */
			deblogfile=fopen("/tmp/import.deblogfile", "a");
			DMLOG("child forked");
			import_all(arcerin, sysname, 0);
			chdir ("/");
			rmdir(tmpname);
			fclose(deblogfile);
			_exit(0);
		}
		return(1);

	} else {
		/* ZCONNECT */

		system_master(ich, sys);
		if (auflegen) return 1;
		bereitstellen();
		files = 1;
		senden_queue = todo;
		todo = NULL;
		while (!auflegen) {
			datei_master(ich, sys);
		}
		time(&now);
		fprintf(stderr, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));
		fprintf(deblogfile, "Anrufdauer: ca. %ld Sekunden online\n", (long)(now-online_start));

		close(modem); DMLOG("close modem");
		aufraeumen();
		exit (0);
	}
	
	return 1;
}
