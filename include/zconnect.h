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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 *  zconnect.h:
 *
 *	Globale Typdefinitionen und Prototypen der Library-Funktionen.
 */

/*
 *    WICHTIG:
 *
 *	hier ist voruebergehend "ZERBERUS51_BUG" definiert, um einen Daten-
 *	austausch mit den auf dem Markt befindlichen ZCONNECT Systemen zu
 *	ermoeglichen.
 *
 *	Dies fuehrt zu einem illegalen Verhalten bei Uebertragungen mit
 *	unidirektionalen Protokollen (x, y, z-modem) und sollte daher entfernt
 *	werden, sobald die lokalen ZCONNECT-Systeme upgedated haben!
 *
 *  Anm.: Das Verhalten ist nicht strikt illegal, es wird lediglich
 *        davon abgeraten. Uebrigens macht es Zerbeus 5.3 Rel. 3.0
 *        immer noch nicht besser.
 */

#define ZERBERUS51_BUG	/* zwei ZMODEM direkt hintereinander sind erlaubt */


#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <setjmp.h>
#include <signal.h>

#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "version.h"
#include "track.h"
#include "line.h"
#include "boxstat.h"
#include "ministat.h"
#include "uulog.h"


/* ---------------------------------------------------------------------------
 *
 *  Die Titelmeldung
 *
 * -------------------------------------------------------------------------*/

#define	TCP_BANNER	\
	"ZCONNECT 3.1 via TCP server - this is " MAILER " v" VERSION "\n" \
	"Copyright " COPYRIGHT "\n" \
	"\n" \
	"Username: "

/* ---------------------------------------------------------------------------
 *
 *  Datenstrukturen
 *
 * -------------------------------------------------------------------------*/

/*
 *  Die Essenz der in der Sysinfo-Phase ausgehandelten Daten
 *  ueber die aktuelle Verbindung wird in folgender Struktur
 *  gespeichert:
 */
typedef struct {
	char	*local_arc,	/* ARCer hier */
		*remote_arc,	/* ARCer der Gegenstelle */
		*proto,		/* Uebertragungsprotokoll */
		*remote_sys,	/* Name der Gegenstelle */
		*tmp_dir,	/* Das Arbeitsverzeichnis */
		*upload_dir;	/* Das Datenverzeichnis (fuer diesen Netcall) */
} connect_t;

/*
 *  Kodierung fuer Aktions-Arten:
 */

#define UNKNOWN		0
#define	MSG_SEND	1
#define MSG_RECV	2
#define	FILE_SEND	3
#define	FILE_RECV	4
#define	MSG_DELETE	5

/*
 *  Kodierung fuer grade:
 */

#define NONE	0
#define PM	1
#define EIL	2
#define BRETT	4
#define FEHLER	8
#define ALL	15

/*
 *  Wir erstellen waehrend der Datenaustausch-Phase in BLK1 und BLK2
 *  eine Liste von zu erledigenden Dingen, die dann von den Low-Level-
 *  Routinen im Uebergang von BLK4 -> BLK1 ausgefuehrt werden.
 */
typedef struct action_st {
	char *dir,		/* chdir() hierhin */
	     *file;		/* Dateiname in diesem Verzeichnis */
	unsigned work;		/* Code fuer die Aktion */
	unsigned grade;		/* Kodierung fuer MSG-Arten (PBEF) */
	int delete;		/* Datei nachher loeschen? */
	struct action_st *next;	/* Das ganze ist eine verketette Liste */
} action_t, *action_p;

/* ---------------------------------------------------------------------------
 *
 *  Globale Variablen
 *
 * -------------------------------------------------------------------------*/

extern connect_t connection;	/* Erst nach Ende der sysinfo-Phase gueltig! */

extern action_p todo,		/* Die globale Warteschlange */
		done,		/* Die erledigte Arbeiten (eventuell noch zu
				 * loeschende Dateien). Wenn der naechste
				 * BLK1 erfolgreich empfangen wurde, duerfen
				 * die mit delete=1 versehenen Dateien geloscht
				 * werden
				 */
		senden_queue;	/* Die Liste der zu versendenden Dateien */

/*
 * auflegen = True -> Dies ist der letzte Paket-Austausch, wir haben uns mit
 * der Gegenseite geeinigt, jetzt aufzuhoeren.
 */
extern int auflegen;

/*
 * auflegen_gesendet = True -> die Gegenseite weiss schon, das wir uns
 * auf auflegen geeinigt haben (LOGOFF: Header wurde verschickt).
 */
extern int auflegen_gesendet;

/*
 * auflegen_empfangen = True -> Die Gegenseit hat LOGOFF: gesendet
 */
extern int auflegen_empfangen;

extern jmp_buf timeout, nocarrier;
extern FILE *deblogfile;
extern int logname;

/*
 * Wartezeit - wenn die Gegenstelle etwas Zeit braucht, bis die Daten da sind.
 */
extern unsigned waittime;

/* ---------------------------------------------------------------------------
 *
 *  Funktionen
 *
 * -------------------------------------------------------------------------*/

/*
 *  Traegt eine Aktion in die todo-Warteschlange ein:
 */
void queue_action(char *dir, char *file, unsigned work, unsigned grade, int delete);

/*
 *  Entfernt eine Aktion aus der Warteschlange (z.B. nach Ablehnung)
 */
void remove_queue(action_p);

/*
 *  Loescht die Aktions-Warteschlange (z.B. nach EXECUTE:N). Liefert
 *  NULL.
 */
action_p clear_queue(action_p);

void prologue(void);
void system_dialog(void);
void datei_dialog(void);
void system_master(header_p hmyself, header_p remote);
void datei_master(header_p hmyself, header_p remote);
void logoff(const char *msg);
char *findport(int port, int hd_code, header_p hd);
int ist_in(char *menge, const char *elem);
char *waehle(char *r_menge, char *l_menge);

void banner_login(void);


/*
 * Zentrale Routine:
 *	Ermittelt Gemeinsamkeiten aus zwei Systembeschreibungen und
 *	setzt als Seiteneffekt die globale Struktur "connection".
 */
header_p sysparam(header_p local, header_p remote, header_p sysfile);
void bereitstellen(void);
void aufraeumen(void);
header_p get_myself(void);
int import_all(const char *arcer, const char *sysname);

/*
 *  Paket-Routinen: phase ist 1 .. 4 (siehe ZCONNECT - Doku)
 */
void ack(int phase);
void nak(void);
header_p get_blk(int phase);
header_p put_blk(header_p block, int phase);
void wait_for_action(header_p eotx);
void request_action(void);
unsigned queue_maske(action_p);
unsigned maske(const char *);
void entmaske(char *ziel, unsigned grade);

/*
 * und fuer Phase 5 .. 6
 */
header_p get_beg(int phase);

int lock_device (int flok, const char *device);

#if ENABLE_MODEM_LOG
#define	DMLOG(msg)	{ fprintf(deblogfile, "#$ %8ld [%d] %s:%d: %s\n", time(NULL), getpid(), __FILE__, __LINE__, msg); fflush(deblogfile); }
#else
#define	DMLOG(msg)	{ /* msg */ }
#endif
