/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1996  Christopher Creutzig
 *  Copyright (C) 1999-2000  Anderas Barth
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
 * JANUS Login fuer UNIX
 *
 *  ACHTUNG: eine sichere Funktion bei der Annahme von JANUS
 *           Anrufen durch ein UNIX System kann derzeit nicht gewaehrleistet
 *           werden!
 */

/*
 *
 * Deutlich Ueberarbeitet und geaendert von
 * Andreas Barth <aba@muenchen.pro-bahn.de> im Herbst 1999
 *
 */

#define _GNU_SOURCE
#include "auth.h"
#include "config.h"
#include "zconnect.h"

#include <sys/types.h>
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#include "utility.h"
#include "xprog.h"
#include "locknames.h"
#include "locknew.h"
#include "calllib.h"

#define	WHO	"JANUS"

#define NAK	0x15
#define ACK	0x06

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

FILE *deblogfile;

static const char *tty = "/dev/tty";

int has_syslock=FALSE, has_ttylock=FALSE, has_data=FALSE;
		/* welche Locks haben wir ? */
static char *spooldir;
		/* Spooldir */

static auth_t* sys; /* Datem zu dem einzelnen System */

static void abbruch(const int cause);
static int getln (char *p, const int anzeige, const int max);

struct call_t {
	const char *sysspooldir; /* sysspooldir */
	const char *tmpdir; /* tmp-Verzeichnis fuer diesen Netcall */
	const char *outname; /* Namen der zu versendenden Datei (voller Pfad) */
	const char *inname; /* Namen der zu empfangenden Datei (voller Pfad) */
	const char *spooloutname; /* Ruhe-Namen der zu versendenden Datei
		(voller Pfad), also spooldir/sys->lsysname.sys->arcerout */
} call;

/* Fuer die Signals */

static void
handle_timeout() {
	abbruch(E_ALARM);
}

static void
handle_nocarrier() {
	abbruch(E_HUP);
}



static int
getln (char *p, const int i, const int y) {
	int j=0;
	int z;
	int max=y;

	while (max > 0) {
		z = getchar();
		if (z == '\r') break;
		if (isprint(z)) {
			j++;
			max--;
			*p++ = z;
			if (i) {
				putc(z, stdout);
			}
		}
	}
	*p = '\0';
	fputs("\r\n", stdout);
	return j;
}




static int init();
static int getsysname(char *sysname, char *passwd, int len);
static int mvtotmp(const auth_t *s);
static int doonline(const auth_t *s);
static int initnames(const auth_t *s);

/*
 *   Hauptprogramm, Gesamtstruktur
 */

int
main(int argc, char **argv)
{
	char sysname[80], passwd[80];
	int ret;

	init();

	tty = ttyname(0);
	lock_device(1, tty);

	getsysname(sysname, passwd, 80);

	if (!(sys = authsys(1, sysname, passwd, systemedir))) {
		fputs("Netzzugriff verweigert.\r\n", stdout);
		newlog(INCOMING, "Falscher Login: %s [%s]", sysname, tty);
		abbruch(0);
	}

	newlog(INCOMING, "System %s erfolgreich eingeloggt [%s]",
		sys->ssysname, tty);

	fputs("Running ARC....\r\n"
	      "Running ARC....\r\n"
	      "Running ARC....\r\n"
	      "Running ARC....\r\n", stdout);

	if ((ret=initnames(sys))) {
		abbruch(ret);
	}

	if (mkdir(call.tmpdir, 0777)) { /* XXX sind die Permissions so gut ? */
		newlog(ERRLOG, "cannot create directory %s: %s",
			call.tmpdir, strerror(errno));
		abbruch(0);
	}
	chdir(call.tmpdir);

	if ((ret=mvtotmp(sys))) {
		abbruch(ret);
	} /* Ok, wir haben Datei zum Versenden und evtl. den Lock */


	if ((ret=doonline(sys))) {
		abbruch(ret);
	}

	/* Fertig, Modem auflegen */
	signal(SIGHUP, SIG_IGN);
	fclose(stderr);
	fclose(stdin);
	hangup(fileno(stdout));
	fclose(stdout);
	lock_device(0, tty);
	/* Netcall war erfolgreich, also Daten loeschen */
	newlog(INCOMING, WHO " Netcall erfolgreich");
	if (has_data)
		backup3(backoutdir,call.outname,sys->ssysname,sys->arcerout);
	else
		remove(call.outname);
	has_data=FALSE;

	rmnewlock(spooldir, sys->lsysname);
	has_syslock=FALSE;

	/*
	 * Und empfangene Daten (im Hintergrund) einlesen,
	 * das Modem wird sofort wieder freigegeben.
	 */
	if (fork() == 0) {
		/* Ich bin child */
		import_all(sys->arcerin, sys->lsysname);
		chdir ("/");
		rmdir(call.tmpdir);
	}
	return 0;
}


/***********************************************
 * Hier kommen die "neuen" Subprogramme        *
 ***********************************************/

static int
init() {
	/* Initalisiert die verschiedenen Dinge */

	initlog("januslogin");

	deblogfile = fopen("/tmp/zlog", "a");
	if (!deblogfile) {
		deblogfile = fopen("/dev/null", "w");
		if (!deblogfile) {
			printf("Sorry - kann Logfile nicht schreiben...\n");
			exit( 10 );
		}
	}


	/*
	 *  Da dieses Programm Login-Shell ist, muss die Fehlerausgabe
	 *  irgendwo hin dirigiert werden...
	 */

/*	freopen("/tmp/januslogin-stderr.log", "w", stderr); */
	freopen("/dev/null", "w", stderr);

	/*
	 *  Konfiguration einlesen.
	 */
	minireadstat();

	spooldir=netcalldir;

	/*
	 *  Verschiedene Signal-Handler setzen
	 */

	signal(SIGHUP, handle_nocarrier);
	signal(SIGALRM, handle_timeout);

	/*
	 *  Hier beginnt der eigentliche Datenaustausch
	 */
	set_rawmode(fileno(stdin));
	set_rawmode(fileno(stdout));


	return 0;
}

static int
getsysname(char *sysname, char *passwd, int len) {
	int Status=0;

	alarm(45);

	while(Status < 2) {
	    switch(Status) {

	      case 0:
		sleep(1);
		fflush(stdin);
		fputs("Systemname: ", stdout);
		fflush(stdout);
		getln(sysname, TRUE, len);

		if (sysname[0] &&
			stricmp(sysname, "zerberus") &&
			stricmp(sysname, "janus") &&
			stricmp(sysname, "janus2")) {
				/* Strich am Anfang des Namens entefernen */
				if ( sysname[ 0 ] == '-' )
					strcpy(sysname,sysname+1);
				strlwr(sysname);
				Status=1;
		}
		break;


	      case 1:
		sleep(1);
		fflush(stdin);
		fputs("Passwort: ", stdout);
		fflush(stdout);
		getln(passwd, FALSE, len);

		if (!stricmp(passwd, "zerberus") ||
		    !stricmp(passwd, "janus") ||
		    !stricmp(passwd, "janus2")) {
			Status=0;
			break;
		}

		/* Das soll auch funktionieren, wenn der - bei einem System
		   ignoriert wurde. */
		if ( passwd[ 0 ] == '-' ) {
			if (stricmp(sysname+1, passwd+1) == 0)
				break;
		} else {
			if (stricmp(sysname, passwd) == 0)
				break;
		}

		Status=2;
		alarm(0);
		break;
	    }
	}
	alarm(0);
	return 0;
}

static int
initnames(const auth_t* s) {
#if HAVE_ASPRINTF
	if (
	   ( asprintf (&(call.tmpdir), "%s/%s.%d.dir",
		spooldir, s->lsysname, getpid()) > 0 ) &&
	   ( asprintf (&(call.sysspooldir), "%s/%s",
		spooldir, s->lsysname) > 0 ) &&
	   ( asprintf (&(call.outname), "%s/called.%s",
		call.tmpdir, s->arcerout) > 0 ) &&
	   ( asprintf (&(call.inname), "%s/caller.%s",
		call.tmpdir, s->arcerin) > 0 ) &&
	   ( asprintf (&(call.spooloutname), "%s/%s.%s",
		spooldir, s->lsysname, s->arcerout) > 0 )
	   ) {
		return 0;
	}

	return -1;
#else
	char *x,*y;

	if (!(x = (char *) malloc(
	    (strlen(spooldir)+strlen(s->lsysname)+100)*sizeof(char)))
			 /* OK, zu hoch gegriffen*/
	 || !(y = (char *) malloc (
	    strlen(spooldir)+strlen(s->lsysname)+2))
	    ) {
		newlog(ERRLOG, "malloc schlaegt fehl");
		return -1; /* Wir kriegen keinen Speicher */
	}
	sprintf(x, "%s/%s.%d.dir", spooldir, s->lsysname, getpid());
	sprintf(y, "%s/%s", spooldir, s->lsysname);
	call.tmpdir=x;
	call.sysspooldir=y;

        if (!(x = (char *) malloc(
            (strlen(call.tmpdir)+strlen(s->arcerout)+10)*sizeof(char)))
            ) {
                newlog(ERRLOG, "malloc schlaegt fehl");
                return -1; /* Wir kriegen keinen Speicher */
        }
	sprintf(x, "%s/called.%s", call.tmpdir, s->arcerout);
	call.outname=x;

        if (!(x = (char *) malloc(
            (strlen(call.tmpdir)+strlen(s->arcerin)+10)*sizeof(char)))
            ) {
                newlog(ERRLOG, "malloc schlaegt fehl");
                return -1; /* Wir kriegen keinen Speicher */
        }
	sprintf(x, "%s/caller.%s", call.tmpdir, s->arcerin);
	call.inname=x;

        if (!(x = (char *) malloc(
            (strlen(spooldir)+strlen(s->arcerout)+
	     strlen(s->lsysname))*sizeof(char)))
            ) {
                newlog(ERRLOG, "malloc schlaegt fehl");
                return -1; /* Wir kriegen keinen Speicher */
        }
	sprintf(x, "%s/%s.%s", spooldir, s->lsysname, s->arcerout);
	call.spooloutname=x;
	return 0;
#endif
}

static int
mvtotmp(const auth_t *s)
 {
	int ret;
	FILE *f;

	if (
	    (!(ret=access(call.spooloutname, R_OK))) /* 0 = access ok */
	 || (!(donewlock(spooldir, s->lsysname))) /* 0 = Lock weg */
	  ) { /* Ok, wir haben den Lock */
		has_syslock=TRUE;
	}

	 if ( !has_syslock || (ret=rename(call.spooloutname, call.outname))
			 /* 0=hat geklappt */
	   ) {
		/* Ok, an die richtige Datei kommen wir nicht */
		/* Wenn wir gelockt haben, sollten wir ihn jetzt zurueckgeben */
		if (has_syslock) {
			rmnewlock(spooldir, s->lsysname);
			has_syslock=FALSE;
		}

		if (!(f=fopen(call.outname, "wb"))) {
			return 1;
		}
		fputs("\r\n", f);
		return(fclose(f));
	} else {
		/* Ok, wir haben die Daten. Jeder Abbruch muss die Daten
		   jetzt wieder zurueckgeben */
		has_data=TRUE;
	}

	return 0;
}

static int
doonline(const auth_t *s) {

	int j,z;
#ifndef DISABLE_JANUS_CHECKSUM
	int i;
	char sernr[6];
#endif

	/*
	 *  Man glaubt es kaum, aber einige Programme schaffen es nicht,
	 *  ein Packzeit von 0 Sekunden der Gegenseite zu verstehen...
	 */
	sleep(1);

	/* Seriennummer "pruefen" */
	alarm(30);
	putchar(NAK); fflush(stdout);
	z = 0;
#ifdef DISABLE_JANUS_CHECKSUM
	for (j=0; j<4; j++) {
		getchar(); /* Die Seriennummer */
	}
#else
	for ( i = 0; i < 5; i++ ) {
		/* Die Pruefsumme der Seriennummer zu pruefen,
		   hilft Zmodem richtig zu starten (dm)
		 */
		for (j=0; j<5; j++) {
			sernr[j] = getchar(); /* Die Seriennummer */
		}
		sernr[5] = sernr[0]+sernr[1]+sernr[2]+sernr[3];
		if ( sernr[5] == sernr[4] )
			break;
	}
#endif
	getchar(); /* Und die Pruefsumme */
	putchar(ACK); /* schon ok ... ;-) */
	fflush(stdout);
	alarm(0);

	if (recvfile(s->transfer, call.inname)) {
		newlog(INCOMING, "%s: failed %s (%s)",
			s->ssysname, call.inname, s->transfer);
		return 1;
	}

	if (sendfile(s->transfer, call.outname)) {
		newlog(INCOMING, "%s: failed %s (%s)",
			s->ssysname, call.outname, s->transfer);
		return 1;
	}

	return 0;
}


static void
abbruch(const int cause) {
	const char *grund;
	int retval;

	alarm(0);

	if ((E_WRONGVERSION <= cause) && (cause <= E_NOPROTO)) {
		grund="Fehler beim Login";
		retval=1;
	} else switch(cause) {

		case E_ALARM:
		grund="Timeout";
		retval=1;
		break;

		case E_HUP:
		grund="Aufgelegt";
		retval=2;
		break;

		case 0:
		grund="";
		retval=1;

		default:
		grund="(unknown)";
		retval=255;
		break;
	}

	/* Lock freigeben */
	lock_device(0, tty);
	if (has_syslock) {
	    if (has_data) {

#if 0
		if (waitnolock(lockname, 180)) {
			newlog(ERRLOG, "Prearc still running %s",
				lockname );
		}
#endif

		/* Jetzt muessen die Daten zurueck aus dem temporaeren
		 * Verzeichnis ins Haupt-Netcall-Verzeichnis
		 */

		if (chdir(call.sysspooldir)) {
			newlog(ERRLOG, "Cant change to %s (%s)",
				call.sysspooldir, strerror(errno));
			exit(1);
		} /* Wieso muessen wir da ueberhaupt rein - ueberpruefen */

		if (link(call.outname, call.spooloutname)) {
		/* link statt rename, rename ueberschreibt einfach, was schon
		 * da ist :-(
		 */

		/*
		 * Der einfache Fall geht nicht: es ist schon was neues da,
		 * wir koennen die ARC Datei nicht an die urspruengliche
		 * Stelle zurueckschieben - dann packen
		 * wir die Daten einfach wieder im Sammel-Verzeichnis aus.
		 * Sie werden dann beim naechsten PreARC Lauf miterfasst.
		 */
			call_auspack(sys->arcerout, call.outname);
		}
		has_data=FALSE;
	    }

            /* keine Daten (mehr), also nur die Datei loeschen */
		if (remove(call.outname)) {
			newlog(ERRLOG, "Cant delete %s (%s)",
				call.outname, strerror(errno));
		} /* Und die Eingangsdatei kann im Prinzip auch loeschen */
#if 0
		if (remove(call.inname)) {
			newlog(ERRLOG, "Cant delete %s (%s)",
				call.inname, strerror(errno));
		}
#endif
	/* Noch das Verzeichnis loeschen */
	    chdir("/");
	    rmdir(call.tmpdir);

	    rmnewlock(spooldir, sys->lsysname);
	    has_syslock=FALSE;
	}

	newlog(INCOMING, "Abbruch: %s", grund);

	exit (retval);
}


