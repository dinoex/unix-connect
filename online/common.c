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
 *  Dirk Meyer, Im Grund 4, 34317 Habichstwald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */


/*
 *  common.c:
 *
 *    Routinen, um Gemeinsamkeiten zwischen zwei ZCONNECT Systemen zu ermitteln
 */

#include "config.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zconnect.h"

connect_t connection;

/*
 *  Sucht im ZCONNECT-Block "hd" den Header "hd_code" und darin
 *  die Spezifikation fuer den gewuenschten "port".
 */
char *findport(int port, int hd_code, header_p hd)
{
	header_p p;
	char *s;
	int nr;

	p = find(hd_code, hd);
	while (p) {
		nr = 0;
		sscanf(p->text, "%d", &nr);
		if (nr == 0 || nr == port) {
			s = p->text;
			/*
			 *  Die Portnummer am Anfang des Strings entfernen...
			 */
			while (isdigit(*s))
				s++;
			while (isspace(*s))
				s++;
			return dstrdup(s);
		}
		p = p->other;
	}

	return NULL;
}

/*
 *   Ist in der Liste "menge" das Element "elem" enthalten?
 */
int ist_in(char *menge, const char *elem)
{
	char *s;

	for (s = menge; (s = strtok(s, " \t;,")) != NULL; s = NULL)
		if (stricmp(s, elem) == 0)
			return 1;

	return 0;
}

/*
 *  r_menge = Liste der remote Eigenschaften
 *  l_menge = Liste der lokalen Eigenschaften
 *    ->      Ein gemeinsamer Eintrag (der erste moegliche nach lokaler Prio)
 */
char *waehle(char *r_menge, char *l_menge)
{
	char *s;

	for (s = l_menge; (s = strtok(s, " \t;,")) != NULL; s = NULL)
		if (ist_in(r_menge, s))
			return s;

	return NULL;
}

/*
 *   Ermittel die Einstellungen fuer die gegebenen Systemdaten.
 *   Bei "local" wird jede portspezifische Information ignoriert,
 *   da unter UNIX kein Grund besteht, z.B. spezielle Packer nur
 *   auf einzelnen Ports zur Verfuegung zu stellen.
 *
 *   Die Portspezifischen Uebertragungsverfahren sind unwahrscheinlich,
 *   aber theoretisch denkbar. Eventuell wird hier dann halt
 *   ueberfluessigerweise "zmodem" aktiviert, obwohl "uucp-e" sinnvoller
 *   waere - was solls...
 *
 *   Ergebnis:
 *
 *	Der naechste zu uebertragende Block - oder NULL, wenn ein Fehler
 *	auftrat. Dieser ist dann bereits per "logoff(..)" angemeldet,
 *	aber noch nicht uebertragen.
 */
header_p sysparam(header_p local, header_p remote, header_p sysfile)
{
	header_p p, erg, d;
	char	*r_arc, *r_arcerout, *r_proto, *domain,
		*my_arc, *my_proto, *my_protos, *proto;
	const char	*my_arcerout, *common_arc;
	char buffer[FILENAME_MAX], fname[FILENAME_MAX], r_sys[FILENAME_MAX];
	int r_port;

	erg = NULL;
	p = find(HD_SYS, remote);
	if (!p) {
		fprintf(stderr, "Gegenseite identifiziert sich nicht!\n");
		fprintf(deblogfile, "Gegenseite identifiziert sich nicht!\n");
		logoff("Protokoll-Fehler: SYS: Header fehlt");
		newlog(ERRLOG, "Gegenseite identifiziert sich nicht");
		return NULL;
	}
	d = find(HD_DOMAIN, remote);
	if (!d) {
		logoff("Protokoll-Fehler: kein " HN_DOMAIN
			": Header (System-Domains)");
		put_blk(NULL, 2);
		newlog(ERRLOG,
			"Protokoll-Fehler: kein " HN_DOMAIN
			": Header (System-Domains)");
		return NULL;
	}
	for (domain = strtok(d->text, " ;:"); domain; domain = strtok(NULL, " ;,")) {
		sprintf(r_sys, "%s.%s", p->text, domain);
		strlwr(r_sys);
		sprintf(fname, "%s/%s", systemedir, r_sys);
		if (access(fname, R_OK) == 0) break;
	}
	if (access(fname, R_OK) != 0) {
		sprintf(r_sys, "%s.%s", p->text, d->text);
		strlwr(r_sys);
	}

	connection.remote_sys = dstrdup(r_sys);
	sprintf(buffer, "%s/zc.%s.%d", netcalldir, r_sys, getpid());
	mkdir(buffer, 0777);
	connection.tmp_dir = dstrdup(buffer);

	r_port = 0;
	p = find(HD_PORT, remote);
	if (p) sscanf(p->text, "%d", &r_port);

	fprintf(stderr, "Verbindung mit %s (Port %d)\n", r_sys, r_port);
	fprintf(deblogfile, "Verbindung mit %s (Port %d)\n", r_sys, r_port);
	newlog(INCOMING, "ZCONNECT Empfang %s Passwort OK", r_sys );

	r_arc = findport(r_port, HD_ARC, remote);
	r_arcerout = findport(r_port, HD_ARCEROUT, remote);
	r_proto = findport(r_port, HD_PROTO, remote);

	/*
	 *  Derzeit haben wir immer nur Port #1 (local)
	 */
	my_arc = findport(1, HD_ARC, local);
	my_protos = findport(1, HD_PROTO, local);
	p = find(HD_ARCEROUT, sysfile);
	my_arcerout = p ? p->text : "ARC";
	p = find(HD_PROTO, sysfile);
	my_proto = p ? p->text : dstrdup( "ZMODEM" );

	if (!ist_in(r_arc, my_arcerout)) {
		/*
		 * Die Gegenseite kann unseren Packer nicht auspacken.
		 * Wir waehlen einen neuen und packen um...
		 */
		 common_arc = waehle(r_arc, my_arc);
		 if (!common_arc) {
		 	fprintf(stderr,
				"FATAL: kein gemeinsamer Packer gefunden!\n");
		 	fprintf(deblogfile,
				"FATAL: kein gemeinsamer Packer gefunden!\n");
		 	logoff("Kein gemeinsamer Packer verfuegbar");
			newlog(ERRLOG,
				"FATAL: kein gemeinsamer Packer gefunden");
		 	return NULL;
		 }
	} else {
		/*
		 *  Der vorgeschlagene Packer remote -> local ist ok.
		 */
		 common_arc = my_arcerout;
	}
	connection.local_arc = dstrdup(common_arc);
	erg = add_header(common_arc, HD_ARCEROUT, erg);
	if (!ist_in(r_proto, my_proto)) {
		proto = waehle(r_proto, my_protos);
		fputs("Unser Protokoll ist drueben unbekannt\n", stderr);
	} else {
		proto = my_proto;
	}
	if (!proto) {
		fprintf(stderr,
			"FATAL: kein gemeinsames Uebertragungsprotokoll!\n");
		fprintf(deblogfile,
			"FATAL: kein gemeinsames Uebertragungsprotokoll!\n");
		logoff("Kein gemeinsames Uebertragungsprotokoll");
		newlog(ERRLOG,
			"FATAL: kein gemeinsames Uebertragungsprotokoll");
		return NULL;
	}
	erg = add_header(proto, HD_PROTO, erg);
	connection.proto = dstrdup(proto);

	if (r_arcerout) {
		/*
		 *  Die Gegenseite hat schon einen Packer vorgeschlagen...
		 */
		 if (!ist_in(my_arc, r_arcerout)) {
		 	erg = add_header(common_arc, HD_ARCERIN, erg);
		 	connection.remote_arc = dstrdup(common_arc);
		 } else {
		 	erg = add_header(r_arcerout, HD_ARCERIN, erg);
		 	connection.remote_arc = dstrdup(r_arcerout);
		 }
	}

	if (r_arc) dfree(r_arc);
	if (r_arcerout) dfree(r_arcerout);
	if (r_proto) dfree(r_proto);
	if (my_arc) dfree(my_arc);
	if (my_protos) dfree(my_proto);

	return erg;
}
