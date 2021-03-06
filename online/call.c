/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1999-2000  Matthias Andree
 *  Copyright (C) 1999-2001  Dirk Meyer
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
 *  call.c
 *
 *  Temporaeres Hilfsprogramm zum Anwaehlen eines anderen Systems.
 *  Dieser Teil (Hauptprogramm) wird spaeter komplett ersetzt.
 *
 *  ACHTUNG: damit dies funktioniert, muss der UUCP Locking-Mechanismus
 *	     korrekt installiert sein. Sieh dazu ../include/policy.h
 *
 */

#include "config.h"
#include "zconnect.h"

#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <fcntl.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#else
#ifndef R_OK
#include <sys/file.h>
#endif
#endif
#ifdef BSD
#include <sys/ioctl.h>	/* setup our own controlling tty */
#endif
#include <sys/wait.h>

#include "utility.h"
#include "hayes.h"
#include "xprog.h"
#include "locknames.h"
#include "calllib.h"

/*
 * Falls das angerufene System einen FIDO-Mailer installiert hat,
 * brechen wir dessen EMSI-Sequenzen ab. Ansonsten wuerden wir bis
 * zum Timeout (10 - 20 s) warten muessen.
 */
#define EMSI_REQ		"**EMSI_REQA77E"	/* vom Fido-Mailer */
#define EMSI_CLI		"**EMSI_CLIF8AC\r" 	/* Abbruchsequenz */

/*
 * Welches Zeilenende soll wr_para verwenden?
 * Da hier nur interne Konfigurationsdateien geschrieben werden, benutzen
 * wir das UNIX-interne Zeilenende.
 */
const char *hd_crlf = "\n";
int logname = OUTGOING;

#define NAK	0x15
#define ACK	0x06

jmp_buf timeout, nocarrier;

FILE *deblogfile;
static time_t online_start;
static int files = 0;

char g_int_prefix[20];	/* international prefix, z.B. 49 fuer Deutschland */
char g_ovst[40];	/* Ortsnetz, z.B. 521 fuer Bielefeld */
void
setup_dial_info(const char *intnl, char *int_prefix, char *ovst, char *telno);

#ifndef O_RDWR
#define O_RDWR	2
#endif

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

static void
usage(void)
{
	fputs (	"Aufruf: zcall (System) (Device) (Speed) [Anzahl der Versuche]\n"
		"um ein ZCONNECT- oder JANUS-System anzurufen\n", stderr);
	exit(1);
}

int anruf( header_p sys, header_p ich, int ttyf, int tries );

char tty[FILENAME_MAX];
int gmodem;

/** dialstring bauen, Default: AT DT */
static int
make_dialstr(char *dialstr, size_t maxlen, char *intntl)
{
	/* input:
	   dialstr: Zielstring
	   maxlen: max. Laenge desselben
	   phone: Rufnummer
	 * return:
	    0: Erfolg
	   -1: Ueberlauf
	   -2: kein %s im Config-Dialstring
	 */
	int rc;
	char phone[60], telno[60], vorw[60], country[60];
	header_p p;

	while (*intntl && !isspace(*intntl))
		intntl++;
	while (*intntl && isspace(*intntl))
		intntl++;
	setup_dial_info(intntl, country, vorw, telno);
	if (strcmp(country, g_int_prefix) == 0) {
		/* nationaler Anruf */
		if (strcmp(vorw, g_ovst) == 0) {
			/* lokaler Anruf */
			rc = snprintf(phone, sizeof(phone), "%s", telno);
		} else {
			rc = snprintf(phone, sizeof(phone), "%s%s%s",
				 fernwahl, vorw, telno);
		}
	} else {
		/* internationaler Anruf */
		rc = snprintf(phone, sizeof(phone), "%s%s%s%s",
			      international ? international:"00",
			      country, vorw, telno);
	}
	if (rc < 0) return -1;

	p = find(HD_MODEM_DIAL, config);
	if(p && !strstr(p->text, "%s")) {
		newlog(ERRLOG, "Konfiguration hat kein %%s im Dialstring!");
		dialstr[0]='\0';
		return -2;
	}
	rc = snprintf(dialstr, maxlen,
		      p ? p->text : "AT DT %s",
		      phone);
	return rc < 0 ? rc : 0;
}

static void
cleanup(void)
{
	if(gmodem>-1)
		restore_linesettings(gmodem);
}

static void
anrufdauer( void )
{
	time_t now;

	if ( online_start == 0 )
		return;

	time(&now);
	newlog(logname,
#ifdef ENABLE_DIFFTIME
		"Anrufdauer: ca. %g Sekunden online",
		difftime(now, online_start));
#else
		"Anrufdauer: ca. %ld Sekunden online",
		(long)(now-online_start));
#endif
	online_start = 0;
}

int maxtry;

int
main(int argc, const char **argv)
{
	const char *sysname, *speed;
	char sysfile[FILENAME_MAX];
	char deblogname[FILENAME_MAX];
	FILE *f;
	header_p sys, ich, p;
	int i;

	initlog("call");

	if (0 == strcmp(argv[0], "-h")) usage();
	if (argc < 4 || argc > 5) usage();

	gmodem=-1;
	atexit(cleanup);

	sysname = strlwr( dstrdup( argv[1] ));
	if (!strchr(argv[2], '/'))
		snprintf(tty, sizeof(tty), "/dev/%s", argv[2]);
	else
		strcpy(tty, argv[2]);

	speed = argv[3];

	maxtry = 1;
	if (argc > 4)
		maxtry = atoi(argv[4]);

	minireadstat();
	sprintf(deblogname, "%s/" DEBUGLOG_FILE, logdir);
	if(debuglevel>0) {
		deblogfile = fopen(deblogname, "w");
		if (!deblogfile) {
			printf("Ich kann das Logfile nicht oeffnen. "
				"Probiere /dev/null...\n");
			deblogfile = fopen("/dev/null", "w");
			if (!deblogfile) {
				printf("Hmm... - /dev/null "
					"nicht schreibbar??\n");
				return 10;
			}
		}
	} else {
		deblogfile = fopen("/dev/null", "w");
		if (!deblogfile) {
			printf("Arghl! - kann /dev/null "
				"nicht zum Schreiben oeffnen!\n\n");
			return 10;
		}
	}
	sprintf(sysfile, "%s/%s", systemedir, sysname);
	f = fopen(sysfile, "r");
	if (!f) {
		perror(sysfile);
		newlog(ERRLOG, "File <%s> not readable: %s",
			sysfile, strerror(errno));
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
		newlog(ERRLOG, "Cannot lock device: %s", tty);
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

	gmodem = open(tty,
#if !defined(__NetBSD__)
			O_RDWR | O_NDELAY
#else
			O_RDWR
#endif
		); DMLOG("open modem");
	if (gmodem < 0) {
		newlog(ERRLOG,
			"Can not access device %s: %s", tty, strerror(errno));
		return 10;
	}
#if !defined(__NetBSD__)
	else {
		/* Nonblock abschalten */
		int n;
		n=fcntl(gmodem, F_GETFL, 0);
		(void)fcntl(gmodem, F_SETFL, n & ~O_NDELAY);
	}
#endif
	save_linesettings(gmodem); DMLOG("saving modem parameters");
	set_rawmode(gmodem); DMLOG("set modem to rawmode");
	set_local(gmodem, 1);
	set_speed(gmodem, speed); DMLOG("set modem speed");
#ifdef TIOCSCTTY
	ioctl(gmodem, TIOCSCTTY, NULL);
#endif

	files = 0;
	online_start = 0;
	fprintf(stderr, "Netcall bei %s [%s]\n", sysname, tty);

	fclose(stdin);
	fclose(stdout);
	dup(gmodem); dup(gmodem); DMLOG("dup modem 2x to stdin and stdout");

	if (setjmp(timeout)) {
		newlog(ERRLOG, "ABBRUCH: Timeout");
		lock_device(0, tty);
		anrufdauer();
		if (files) aufraeumen();

		return 11;
	}
	if (setjmp(nocarrier)) {
		if (!auflegen) {
			newlog(ERRLOG, "ABBRUCH: Gegenstelle hat aufgelegt");
		}
		lock_device(0, tty);
		anrufdauer();
		if (files) aufraeumen();

		return auflegen ? 0 : 12;
	}
	signal(SIGHUP, handle_nocarrier);
	signal(SIGALRM, handle_timeout);
	setup_dial_info(ortsnetz, g_int_prefix, g_ovst, NULL);

	p = find(HD_TEL, sys);
	if (!p) {
		newlog(ERRLOG,
			"Keine Telefonnummer fuer %s gefunden",
			sysname);
		return 2;
	}

	anruf(sys, ich, gmodem, maxtry);

	if (online_start) {
		anrufdauer();
	} else {
		fprintf(stderr, "Keine Verbindung hergestellt.\n");
	}

	lock_device(0, tty);
	/* Device ist freigegeben, aber wir warten noch auf das Ende
	   der Importphase, bevor wir zurueckkehren. */
	wait(NULL);
	return 0;
}

/*
 *  Fuer "verfahren" :
 */
#define JANUS		0
#define ZCONNECT	1

static const char *verf[] = { "JANUS", "ZCONNECT", NULL };
static const char *logstr[] = { "JANUS\r", "zconnect\r" };

/* (1 s pause) +++ (1 s pause) ATH0 <LF>
 * ans Modem schicken und zweimal auf OK warten */
static int
hayes_hangup(int lmodem)
{
	/* returns:
	 * 0: ok
	 * 1: Fehler bei +++
	 * 2: Fehler bei ATH0
	 */
	int t_nocarr;
	int t_ok;
	int t_error;
	int ok;

	free_all_tracks();
	t_nocarr = init_track("NO CARRIER");
	t_ok = init_track("OK");
	t_error = init_track("ERROR");

	alarm(10);
	sleep(1);
	write( lmodem, "+++", 3 );
	sleep(1);
	ok = do_hayes("", lmodem);
	if(ok != t_ok) {
		return 1;
	}
	ok = do_hayes("ATH0", lmodem);
	if(ok != t_ok) {
		return 2;
	}
	return 0;
}

static int
login(int lmodem, int verfahren, const char *myname, const char *passwd)
{
        int     t_ogin, t_ame, t_wort, t_word, t_begin, found,
		t_esc, t_sysname, t_arc, t_verweigert,
		err;
	char	z, pw[40];

	if (verfahren == ZCONNECT)
		strcpy(pw, "0zconnec\r");
	else
		sprintf(pw, "%s\r", passwd);

	free_all_tracks();
        t_esc = init_track(EMSI_REQ);
	t_ogin = init_track("OGIN:");
	if (verfahren == ZCONNECT)
		t_ame = init_track("AME");
	else
		t_ame = init_track("USERNAME:");
	t_wort = init_track("WORT");
	t_word = init_track("WORD");
	t_sysname = -1;
	t_begin = -1;
	t_arc = -1;
	t_verweigert = -1;

        alarm(2*60);
	while (1)
	{
		do { } while (read(lmodem, &z, 1) != 1);
		found = track_char(toupper(z));
		if (!isascii(z) || !iscntrl(z)) {
			putc(z, stderr);
			fflush(stderr);
		} else if (z == '\r')
			putc('\n', stderr);
		if (found == -1) continue;
		if (found == t_esc) {
                        write(lmodem, EMSI_CLI, sizeof(EMSI_CLI));
			flush_modem(lmodem);
                } else if (found == t_ogin || found == t_ame) {
#ifdef SLOW_MODEM
                	sleep(2);	/* fflush auf der Gegenseite? */
#endif
			write(lmodem, logstr[verfahren],
				strlen(logstr[verfahren]));
			flush_modem(lmodem);
			if (verfahren < ZCONNECT && t_sysname == -1)
				t_sysname = init_track("SYSTEMNAME:");
                } else if (found == t_word || found == t_wort) {
			write(lmodem, pw, strlen(pw));
			flush_modem(lmodem);
			if (verfahren == ZCONNECT && t_begin == -1) {
				t_begin = init_track("BEGIN");
			}else {
				t_arc = init_track("ARC");
				t_verweigert = init_track("VERWEIGER");
			}
		} else if (found == t_sysname) {
			write(lmodem, myname, strlen(myname));
			write(lmodem, "\r", 1);
			flush_modem(lmodem);
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
void
setup_dial_info(const char *intnl, char *int_prefix, char *ovst, char *telno)
{
	const char *q;
	char *z;

	if (int_prefix) *int_prefix = '\0';
	if (ovst) *ovst = '\0';
	if (telno) *telno = '\0';

	if (!intnl) return;

	if (!int_prefix) return;
	q=(const char *)intnl; z=int_prefix;
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

static int
janus_wait(int lmodem)
{
	/* FIXME: erkennt Protokollfehler nicht */
	char c;

	fprintf(stderr, "....\n");
	alarm(10*60);
	do {
		while (read(lmodem, &c, 1) != 1) {
			newlog(DEBUGLOG,
				"can't read from modem: %s", strerror(errno));
			return 1;
		}
	} while (c != NAK);
	alarm(0);
	fprintf(stderr, "Gegenseite hat gepackt\n");

	alarm(15);
	do {
		write(lmodem, "UNIXD", 5);
		while (read(lmodem, &c, 1) != 1) {
			newlog(DEBUGLOG,
				"can't read from modem: %s", strerror(errno));
			return 1;
		}
	} while (c != ACK);
	alarm(0);
	fprintf(stderr, "Seriennummern OK\n");
	return 0;
}

int
anruf( header_p sys, header_p ich, int lmodem, int tries )
{
	char dialstr[80];
	char lockname[FILENAME_MAX];
	header_p p;
	char *name, *pw;
	const char **v;
	int i, err;
	int dial_cnt = 1;

	/* fuer Janus */
	char *arcer=NULL, *arcerin=NULL, *transfer=NULL, *domain;
	header_p t, d;
	int netcall_error;
	char filename[FILENAME_MAX];
	char tmpname[FILENAME_MAX];
	char outname[FILENAME_MAX];
	char inname[FILENAME_MAX];
	char sysname[FILENAME_MAX];
	struct stat st;
	/* ende fuer Janus */

	t = find(HD_ARCEROUT, sys);
	if (!t) {
		newlog(ERRLOG,
			"Kein ausgehender Packer definiert");
		return 1;
	}
	arcer = t->text;
	strlwr(arcer);

	t = find(HD_ARCERIN, sys);
	if (!t) {
		newlog(ERRLOG,
			"Kein eingehender Packer definiert");
		return 1;
	}
	arcerin = t->text;
	strlwr(arcerin);

	t = find(HD_PROTO, sys);
	if (!t) {
		newlog(ERRLOG,
			"Kein Uebertragungsprotokoll definiert");
		return 1;
	}
	transfer = t->text;
	strlwr(transfer);

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

	if (i < ZCONNECT) {
		t = find(HD_SYS, sys);
		if (!t) {
			newlog(ERRLOG,
				"Illegale Systemdatei: Kein " HN_SYS
				": Header oder falscher Name: %s", filename);
			return 1;
		}
		d = find(HD_DOMAIN, sys);
		if (!d) {
			newlog(ERRLOG,
				"Illegale Systemdatei: Kein " HN_DOMAIN
				": Header: %s", filename);
			return 1;
		}

		for (domain = strtok(d->text, " ;,:");
				domain;
				domain = strtok(NULL, " ;,:")) {
			sprintf(sysname, "%s.%s", t->text, domain);
			strlwr(sysname);
			sprintf(tmpname, "%s/%s", netcalldir, sysname);
			newlog(logname,
				"Suche Verzeichnis, versuche %s...", tmpname);
			if (access(tmpname, R_OK|X_OK) == 0)
				break;
		}

		if(access(tmpname, R_OK|X_OK)) {
			/* Problem: temp. Verzeichnis nicht zu haben */
			newlog(logname,
				"Problem beim Netcall: Verzeichnis "
				"nicht gefunden");
			return 1;
		}
	}

	/* ##### HIER WIRD ANGEWAEHLT ##### */
	p = find(HD_TEL, sys);
	while(tries) {
		if(!p) p = find(HD_TEL, sys);
		make_dialstr(dialstr, sizeof(dialstr), p->text);

		fprintf(stderr, "%3d. Anwahlversuch (noch %d): %-.25s\n",
			dial_cnt++, tries, dialstr);                     

		if (redial(dialstr, lmodem, 1) != 0 ) {
			tries--;
			p = p->other;
		} else {
			/* connect */
			break;
		}
	}

	set_local(lmodem, 0);
	time(&online_start);

	if (i < ZCONNECT) {
		if (name) dfree(name);
		name = dstrdup(boxstat.boxname);
		strupr(name);
#ifdef ENABLE_CAPS_IN_PASSWORD
		strupr(pw);
#endif
	}
	if(login(lmodem, i, name, pw)) return 0;

	if (i < ZCONNECT) { /* JANUS */
		int have_file = 0;

		netcall_error = 0;

		if ( janus_wait(lmodem) != 0 )
			return 0;

		sprintf(tmpname, "%s/%s.%d.dir", netcalldir, sysname, getpid());
		mkdir(tmpname, 0755);
		chdir(tmpname);
		/* outname: ausgehendes Archiv
		 * filename: */
		sprintf(outname, "%s/caller.%s", tmpname, arcer);
		sprintf(filename, "%s/%s.%s", netcalldir, sysname, arcer);
		sprintf(lockname, "%s/%s/" PREARC_LOCK, netcalldir, sysname);
		if (access(filename, R_OK) != 0) {
			FILE *f;
			if(access(filename, F_OK) == 0) {
				newlog(ERRLOG,
					"Leerer Puffer, weil keine Erlaubnis "
					"zum Lesen von %s: %s",
					outname, strerror(errno));
			}

			f = fopen(outname, "wb");
			if (!f) {
				newlog(ERRLOG,
					"Kann Netcall %s nicht erzeugen: %s",
					outname, strerror(errno));
				fclose(deblogfile);
				return 1;
			}
			fputs("\r\n", f);
			fclose(f);
		} else { /* can read filename */
			if (waitnolock(lockname, 180)) {
				fclose(deblogfile);
				newlog(OUTGOING, "System %s Prearc LOCK: %s",
					sysname, lockname );
				return 1;
			}
			fprintf(stderr, "Link: %s -> %s\n",
				filename, outname);
			if(link(filename, outname)) {
				fclose(deblogfile);
				newlog(ERRLOG,
					"Linken: %s -> %s fehlgeschlagen: %s",
					filename, outname, strerror(errno));
				netcall_error = 1;
				goto finish;
			}
			have_file = 1;
		}
		sprintf(inname, "called.%s", arcer);

		st.st_size = 0;
		if(stat(outname, &st)) {
			fprintf(stderr,
				"Zugriff auf %s fehlgeschlagen: %s\n",
				outname, strerror(errno));
			netcall_error = 1;
			goto finish;
		}

		newlog(logname, "Sende %s (%ld Bytes) per %s",
		       outname, (long)st.st_size, transfer);

		err = sendfile(transfer, outname);
		if (err) {
			newlog(logname,
				"Versand der Daten fehlgeschlagen");
			netcall_error = 1;
			goto finish;
		}

		newlog(logname, "Empfange mit %s", transfer);
		if (recvfile(transfer, inname)) {
			newlog(logname,
				"Empfang der Daten fehlgeschlagen");
			netcall_error = 1;
			goto finish;
		}
		st.st_size = 0;
		if(stat(inname, &st)) {
			newlog(logname,
				"Zugriff auf %s fehlgeschlagen: %s",
				inname, strerror(errno));
		}
		newlog(logname, "%ld Bytes empfangen", (long)st.st_size);


	finish:

		/* Fertig, Modem auflegen */
		signal(SIGHUP, SIG_IGN);
		fclose(stdin);
		fclose(stdout);
		hayes_hangup(lmodem); DMLOG("hayes hangup modem");
		hangup(lmodem); DMLOG("hangup modem");
		anrufdauer();

		restore_linesettings(lmodem);
		DMLOG("restoring modem parameters");
		close(lmodem); lmodem=-1; DMLOG("close modem");
		/* neuer stdin */
		fopen("/dev/null", "r");
		/* stderr wird in stdout kopiert */
		dup2(fileno(stderr),fileno(stdout));

		if(!netcall_error) {
			/* Netcall war erfolgreich, also Daten loeschen */
		if(have_file) {
			/* Backups von Nullpuffern sind
			   uninteressant */
			backup3(backoutdir,filename,sysname,arcer);
		}
		/* das ist nur ein Link, den putzen wir weg */
		if ( unlink(outname)) {
			newlog(ERRLOG,
			       "Loeschen von %s fehlgeschlagen: %s",
			       outname, strerror(errno));
		}

		fclose(deblogfile);

		/*
		 * Und empfangene Daten (im Hintergrund) einlesen,
		 * das Modem wird sofort wieder freigegeben.
		 */
		switch(fork()) {
		case -1:
			{ /* cannot fork */
				perror("forking import");
				newlog(ERRLOG,
				       "Forken des Importteils "
				       "fehlgeschlagen: %s",
				       strerror(errno));
				break;
			}
		case 0:
			{
				/* Ich bin child */
				deblogfile=fopen("/tmp/import.deblogfile", "a");
				DMLOG("child forked");
				import_all(arcerin, sysname);
				chdir ("/");
				if(rmdir(tmpname)) {
					newlog(ERRLOG,
						"Loeschen von %s "
						"fehlgeschlagen: %s",
						tmpname, strerror(errno));
				}
				fclose(deblogfile);
				exit(0);
			}
		default: /* parent */
			break;
		}
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
		anrufdauer();
		close(lmodem); DMLOG("close modem");
		aufraeumen();
		exit (0);
	}

	return 1;
}

