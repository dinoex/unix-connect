/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */


/*
 *  pingpong.c:
 *
 *     Paket-Datenaustausch Routinen
 */

#include "config.h"
#include "zconnect.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "utility.h"

int auflegen = 0;
int auflegen_gesendet = 0;
int auflegen_empfangen = 0;

action_p todo = NULL;		/* Globale Warteschlange von Aktionen */
action_p done = NULL;		/* Erledigte Aktionen, Dateien loeschen */

static char *auflegen_msg = NULL;

/*
 *   logoff(msg): traegt einen wartenden LOGOFF: Header mit der <msg> ein,
 *		  der gesendet wird, sobald im ping-pong Platz dafuer ist.
 *		  "auflegen" wird gesetzt, "auflegen_gesendet" erst, wenn der
 *		  Header wirklich verschickt wurde.
 */
void
logoff(const char *msg)
{
	auflegen = 1;
	auflegen_gesendet = 0;
	auflegen_msg = dstrdup(msg);
}

/*
 *   send_ack(phase) :	sendet ein Acknowledge fuer einen korrekt empfangenen
 *			Block aus der angegebenen Phase [1 .. 4]
 */
static void
send_ack(int phase)
{
	char tmp[5];
	header_p p;

	sprintf(tmp, "ACK%d", phase);
	p = new_header(tmp, HD_STATUS);
	wr_packet(p, stdout);
	free_para(p);
}

/*
 *   send_tme(phase) :	sendet ein Transmission End fuer einen korrekt
 *			gesendeten Block aus der angegebenen Phase [1 .. 4]
 */
static void
send_tme(int phase)
{
	char tmp[5];
	header_p p;

	sprintf(tmp, "TME%d", phase);
	p = new_header(tmp, HD_STATUS);
	wr_packet(p, stdout);
	free_para(p);
}

/*
 *   send_nak() : sendet ein Negative-Acknowledge fuer einen fehlerhaft
 *                empfangenen Block und fordert damit den Sender zum
 *		  Retransmit auf. ZCONNECT (in diesem Teil der Online-
 *		  Phase) hat ein Empfangsfenster von 1, d.h. der Empfaenger
 *		  weiss immer genau, welches Paket jetzt vom Sender kommen
 *		  muss.
 */
static void
send_nak(void)
{
	header_p p;

	p = new_header("NAK0", HD_STATUS);
	wr_packet(p, stdout);
	free_para(p);
}

/*
 *  get_blk(int phase) :  Empfange die BLK(phase) Daten
 */
header_p
get_blk(int phase)
{
	header_p p, c, s, last;
	char tme[6], blk[6];
	crc_t crc;
	unsigned int crc_hd;

	/*
	 *  Erwartete Blockarten (wegen Receive Window = 1 koennen hier
	 *  nur diese beiden oder NAK0 erscheinen).
	 */
	sprintf(tme, "TME%d", phase);
	sprintf(blk, "BLK%d", phase);

	last = NULL; p = NULL;
	while (1) {
		if (p) free_para(p);

		/*
		 *  Block einlesen
		 */
		p = rd_packet(stdin, &crc);

		c = find(HD_CRC, p);
		if (!c) {
			fputs("Paket ohne CRC empfangen...\n", deblogfile);
			continue;
		}

		sscanf(c->text, "%x", &crc_hd);
		if (crc != crc_hd) {
			fputs("Paket mit ungueltigem CRC empfangen...\n",
				deblogfile);
			send_nak();
			continue;
		}

		c = find(HD_LOGOFF, p);
		if (c) {
			fprintf(deblogfile,
				"Logoff-Anforderung durch "
				"die Gegenseite:\n\t\"%s\"\n\n",
				c->text);
			auflegen_empfangen = 1;
			auflegen = 1;
			send_ack(phase);
			return p;
		}
		s = find(HD_STATUS, p);
		if (!s) {
			fputs("Paket ohne STATUS empfangen...\n",
				deblogfile);
			continue;
		}

		if (phase == 4)
			fprintf(deblogfile, "[Phase 4] STATUS: %s\n", s->text);

		if (phase == 4 && (stricmp(s->text, "EOT4") == 0 ||
		    stricmp(s->text, "EOT5") == 0 ||
		    stricmp(s->text, "EOT6") == 0)) {
		    	fputs("Rufe wait_for_action\n", deblogfile);
			wait_for_action(p);
			if (p) free_para(p);
			return last;
		} else if (stricmp(s->text, blk) == 0) {
			send_ack(phase);
			if (last) free_para(last);
			last = p;
			p = NULL; /* p (und damit last) nicht freigeben */
			continue;
		} else if (stricmp(s->text, tme) == 0) {
			if (p) free_para(p);
			return last;
		} else if (stricmp(s->text, "NAK0") == 0) {
			fprintf(deblogfile,
				"NAK0 empangen beim Warten auf %s\n", blk);
			/*
			 *  Wir sollen wiederholen. Aber was?
			 *  Eigentlich wollen wir doch Daten kriegen...
			 */
			if (last)
				send_ack(phase); /* Ist sicher OK */
			else
				send_nak(); /* Das ist schon etwas komisch */
			continue;
		}
	}
}

/*
 *  put_blk(int phase) :  Sende die BLK(phase) Daten
 */
header_p
put_blk(header_p block, int phase)
{
	header_p p, c, s;
	char blk[6], lack[6];
	crc_t crc;
	unsigned int crc_hd;

	/*
	 *  Erwartete Antworten:
	 */
	sprintf(blk, "BLK%d", phase);
	sprintf(lack, "ACK%d", phase);

	/*
	 *  BLK-Nr. in das Paket eintragen
	 */
	block = add_header(blk, HD_STATUS, block);
	/*
	 *  Wartet eine Fehlermeldung auf Transport?
	 */
	if (auflegen_msg) {
		block = add_header(auflegen_msg, HD_LOGOFF, block);
		auflegen_gesendet = 1;
		dfree(auflegen_msg);
		auflegen_msg = NULL;
		signal(SIGHUP, SIG_IGN);
	}

	/*
	 *  Paket erstmalig verschicken
	 */
	wr_packet(block, stdout);

	p = NULL;
	while (1) {

		if (p) free_para(p);

		/*
		 *  Bestaetigung fuer das Paket empfangen
		 */
		p = rd_packet(stdin, &crc);

		c = find(HD_CRC, p);
		if (!c) {
			fputs("Paket ohne CRC empfangen...\n", deblogfile);
			continue;	/* Ungueltige Stoerungen... */
		}

		/*
		 *  Bestaetigung fehlerfrei empfangen?
		 */
		sscanf(c->text, "%x", &crc_hd);
		if (crc != crc_hd) {
			fputs("Antwort mit ungueltigem CRC...\n", deblogfile);
			/* xxx
			 *  Hier sollte man vielleicht send_nak() schicken?
			 */
			 continue;
		}

		s = find(HD_STATUS, p);
		if (!s) {
			fputs("Antwort ohne STATUS empfangen...\n",
				deblogfile);
			/* xxx
			 *  Auch hier vielleicht ein send_nak() ?
			 */
			continue;
		}

		if (stricmp(s->text, lack) == 0)
			break;	/* Das Paket wurde positiv bestaetigt... */

		/*
		 *  Eine gueltige Antwort wurde empfangen, aber es war
		 *  keine positive Bestaetigung. Bei NAK0 senden wir den
		 *  Block nochmal, ansonten geht es weiter...
		 */
		if (stricmp(s->text, "NAK0") == 0)
			wr_packet(block, stdout);
	}

	if (p) free_para(p);

	/*
	 *  Dieser Block ist erfolgreich uebertragen, also geht das
	 *  ping-pong eine Stufe weiter. Falls etwas auszufuehren ist,
	 *  wird jetzt mit EOT4-EOT4-EOT4 "Auszeit" beantragt, andernfalls
	 *  wird TMEx gesendet.
	 */
	if (phase == 4 && todo) {
		/*
		 *  Nur am Ende von BLK4 kann eine externe Aktion eingefuegt
		 *  werden.
		 */
		request_action();
	} else
		send_tme(phase);

	/*
	 *  Hier wurde in "block" der STATUS: Header eingefuegt, also
	 *  auch wieder entfernen.
	 */
	block = del_header(HD_STATUS, block);

	return block;
}

/*
 *   send_eot(phase) :	sendet ein Acknowledge fuer einen korrekt empfangenen
 *			Block aus der angegebenen Phase [5 .. 6] (3 Bloecke)
 */
static void
send_eot(int phase)
{
	char tmp[5];
	header_p p;

	sprintf(tmp, "EOT%d", phase);
	p = new_header(tmp, HD_STATUS);
	wr_packet(p, stdout);
	sleep(1);
	wr_packet(p, stdout);
	sleep(1);
	wr_packet(p, stdout);
	free_para(p);
}

/*
 *  get_beg(int phase) :  Empfange die BEG(phase) Daten
 */
header_p
get_beg(int phase)
{
	header_p p, c, s, last;
	char beg[6];
	crc_t crc;
	unsigned int crc_hd;

	/*
	 *  Erwartete Blockarten (wegen Receive Window = 1 koennen hier
	 *  nur diese oder NAK0 erscheinen).
	 */
	sprintf(beg, "BEG%d", phase);

	last = NULL; p = NULL;
	while (1) {
		if (p) free_para(p);

		/*
		 *  Block einlesen
		 */
		p = rd_packet(stdin, &crc);

		c = find(HD_CRC, p);
		if (!c) {
			fputs("Paket ohne CRC empfangen...\n", deblogfile);
			continue;
		}

		sscanf(c->text, "%x", &crc_hd);
		if (crc != crc_hd) {
			fputs("Paket mit ungueltigem CRC empfangen...\n",
				deblogfile);
			send_nak();
			continue;
		}

		c = find(HD_LOGOFF, p);
		if (c) {
			fprintf(deblogfile,
				"Logoff-Anforderung durch "
				"die Gegenseite:\n\t\"%s\"\n\n",
				c->text);
			auflegen_empfangen = 1;
			auflegen = 1;
			send_ack(phase);
			return p;
		}
		s = find(HD_STATUS, p);
		if (!s) {
			fputs("Paket ohne STATUS empfangen...\n", deblogfile);
			continue;
		}

		if (stricmp(s->text, beg) == 0) {
			send_eot(phase);
			return p;
		} else if (stricmp(s->text, "NAK0") == 0) {
			fprintf(deblogfile,
				"NAK0 empangen beim Warten auf %s\n", beg);
			/*
			 *  Wir sollen wiederholen. Aber was?
			 *  Eigentlich wollen wir doch Daten kriegen...
			 */
			if (last)
				send_ack(phase); /* Ist sicher OK */
			else
				send_nak(); /* Das ist schon etwas komisch */
			continue;
		}
	}
}
