/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
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
 *  action.c:
 *
 *	Verwaltung der Warteschlange sowie Ausfuehren der gemerkten Aktionen
 *	in der EOT4 ... EOT6 "Auszeit".
 */

#include "config.h"
#include "zconnect.h"
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include <time.h>
#include "lib.h"
#include "xprog.h"

unsigned waittime;

/*
 *  Traegt eine Aktion in die Warteschlange ein:
 */
void queue_action(char *dir, char *file, unsigned work, unsigned grade, int delete)
{
	action_p new;

	new = dalloc(sizeof(action_t));
	new->work = work;
	new->grade = grade;
	new->delete = delete;
	new->dir = dir ? dstrdup(dir) : NULL;
	new->file = file ? dstrdup(file) : NULL;
	new->next = todo;

	todo = new;
}

/*
 *  Loescht die Aktions-Warteschlange (z.B. nach EXECUTE:N). Setzt todo auf
 *  NULL.
 */
action_p clear_queue(action_p atodo)
{
	action_p p, next;

	for (p=next=atodo; p; p=next) {
		next = p->next;
		if (p->dir) dfree(p->dir);
		if (p->file) dfree(p->file);
		dfree(p);
	}

	return NULL;
}

void remove_queue(action_p p)
{
	for ( ; p; p = p->next)
	   if (p->delete) {
   		fprintf(deblogfile, "Loesche: %s/%s\n", p->dir, p->file);
	   	chdir(p->dir);
	   	remove(p->file);
	   }
}

void entmaske(char *ziel, unsigned grade)
{
	if (grade & PM) *ziel++ = 'P';
	if (grade & EIL) *ziel++ = 'E';
	if (grade & BRETT) *ziel++ = 'B';
	if (grade & FEHLER) *ziel++ = 'F';
	*ziel = '\0';
}

unsigned maske(const char *p)
{
	unsigned erg = NONE;

	while (*p) {
	  switch (*p++) {
	    case 'P': erg |= PM; break;
	    case 'E': erg |= EIL; break;
	    case 'B': erg |= BRETT; break;
	    case 'F': erg |= FEHLER; break;
	  }
	}

	return erg;
}

unsigned queue_maske(action_p p)
{
	unsigned erg;

	for (erg = NONE; p; p = p->next)
		erg |= p->grade;

	return erg;
}

void do_transfers(void);
void do_transfers(void)
{
	action_p doit, last;

	alarm(0);
	for (last = NULL, doit = todo; doit; last = doit, doit = doit->next) {
	  if (doit->work == MSG_RECV || doit->work == FILE_RECV) {
	  	int erg;
	  	char fname[FILENAME_MAX];

	  	chdir(doit->dir ? doit->dir : "/tmp");
	  	if (doit->file)
	  		strcpy(fname, doit->file);
	  	else
	  		sprintf(fname, "rcv%ld", (long)time(NULL));
	  	fprintf(stderr, "EMPFANGEN %s\n", connection.proto); fflush(stderr);
		if ( waittime > 0 )
		{
			header_p dummy;

			if(waittime > 10)
			{ /* wir warten zehn Sekunden weniger, um das BEG5
			     nicht zu verpassen. */
				sleep(waittime-10);
			}
			dummy = get_beg(5);
			free_para(dummy);
		}
		erg = recvfile(connection.proto, fname);

		fprintf(stderr, "\n  ---> %d\n", erg);
		fprintf(deblogfile, "RECEIVE %s --> %d\n", connection.proto, erg);
		logfile(logname, connection.proto, "EMPFANG", "-", erg ? "FEHLER\n" : "OK\n");
		if (erg) logoff("Fehler beim Protokolltransfer");
		break;
	  }
	  if (doit->work == MSG_SEND || doit->work == FILE_SEND) {
	  	int erg;

	  	chdir(doit->dir ? doit->dir : "/tmp");
		fprintf(stderr, "SENDEN %s\n", connection.proto); fflush(stderr);
		erg = sendfile(connection.proto, doit->file ? doit->file : "");

		fprintf(stderr, "\n  ---> %d\n", erg);
		fprintf(deblogfile, "SEND %s --> %d\n", connection.proto, erg);
		logfile(logname, connection.proto, "VERSAND", "-", erg ? "FEHLER\n" : "OK\n");
		if (erg) logoff("Fehler beim Protokolltransfer");
		break;
	  }
	}
	if (doit) {
		/*
		 *  Wir haben 'was getan, also raus aus der Warteschlange,
		 *  rein in die Loesch-Queue.
		 */
		if (!last) {
			/*
			 *  Einfach: erstes Element der Warteschlange
			 */
			last = todo;
			todo = todo->next;
			last->next = done;
			done = last;
		} else {
			/*
			 *  Irgendwo mittendrin, last ist der Vorgaenger
			 */
			last->next = doit->next;
			doit->next = done;
			done = doit;
		}
	}
}

/*
 *  Wird von der get_blk() aufgerufen, wenn die Gegenseite eine Aktion
 *  einleitet. Der uebergebene Header ist der erste EOTx Header, zwei
 *  weitere gleichartige sollten folgen, was aber hier nicht geprueft wird,
 *  sie koennten auch Uebertragungsstoerungen zum Opfer gefallen sein
 *  (wir sind hier auf Low-Level Ebene und koennen die Bloecke nur passiv
 *  gegen ihren CRC pruefen, aber kein Retransmit erzwingen).
 */

int wait_sec = -1;

void wait_for_action(header_p eotx)
{
	crc_t crc;
	header_p p;

	if (wait_sec>0) {
		fputs("Sende EOT5\n", deblogfile);
		p = new_header("EOT5", HD_STATUS);
		wr_packet(p, stdout);
	}
	else
	if (wait_sec==-1) {
		fputs("Sende BEG5\n", deblogfile);
		p = new_header("BEG5", HD_STATUS);
		wr_packet(p, stdout);
	}
	/*
	 *  Wir haben die Uebertragung beim ersten EOT4 getriggert,
	 *  jetzt sollten noch zwei weitere kommen. Die bringen einige
	 *  sz/rz ziemlich durcheinander, daher warten wir die noch
	 *  ab.
	 */
	p = rd_packet(stdin, &crc);
	fprintf(deblogfile, "[Ignoriere Paket:]\n");
	wr_para(p, deblogfile);

	p = rd_packet(stdin, &crc);
	fprintf(deblogfile, "[Ignoriere Paket:]\n");
	wr_para(p, deblogfile);

	do_transfers();
}


/*
 *  Wird von put_blk() nach Uebertragung von BLK4 aufgerufen, in dem
 *  ein EXECUTE:Y enthalten war. Wir senden drei EOTx Bloecke, um die
 *  Gegenseite in den Uebertragungsmodus zu bringen, wobei dazwischen
 *  je eine Sekunde Pause gemacht wird, um Stoerimpulsen nicht die Chance
 *  zu geben, alle drei Bloecke unkenntlich zu machen.
 */
void request_action(void)
{
	header_p p;

	p = new_header("EOT4", HD_STATUS);
	fputs("Sende EOT4-EOT4-EOT4\n", deblogfile);
	wr_packet(p, stdout); sleep(1);
	wr_packet(p, stdout); sleep(1);
	wr_packet(p, stdout);
	free_para(p);
	/*
	 *  Wir haben vorher auf jeden Fall WAIT:0 gesendet, also muessen
	 *  wir jetzt auch nicht "Warten auf Aktion" spielen, es geht gleich
	 *  mit der Dateiuebertragung los.
	 */

	do_transfers();
}
