/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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
 *  christopher@nescio.foebud.org or snail-mail:
 *  Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
 *
 *  There is a mailing-list for user-support:
 *   unix-connect@mailinglisten.im-netz.de,
 *  to join, ask Nora Etukudo at
 *   nora.e@mailinglisten.im-netz.de
 *
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib.h"
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif

char *str2eda(char *text, int *tz_hour, int *tz_min)
{
	struct tm t;
	time_t dt;
	char s_or_w;
	char *answer;
	char buffer[24];
	
	answer=dalloc(sizeof(char)*84*10);

	*tz_hour = *tz_min = 0;
	memset(&t, 0, sizeof t);
	sscanf(text, "%04d%02d%02d%02d%02d%02d%c%d:%d",
		&t.tm_year, &t.tm_mon, &t.tm_mday,
		&t.tm_hour, &t.tm_min, &t.tm_sec,
		&s_or_w, tz_hour, tz_min);
		
	t.tm_year -= 1900;
	t.tm_mon -= 1;

/*
 * Das hier führt aus irgendeinem Grund dazu, daß immer
 * Sonntag herauskommt...
 *
 *	if(s_or_w=='S' || s_or_w=='S')
 *		t.tm_isdst = 1;
 *	else
 *		t.tm_isdst = 0;
 */

/*
        Ziel: die lokale Zeit des Absenders

        Wir bilden mit mktime einen Zeitwert, als ob es unsere
        eigene lokale Zeit waere. Dies liefert uns einen Fehler
        um die Differez der Zeitzonen von Absender und Gate.
	In der Winterzeit gibt es keine Probleme, aber fuer
	ein Datum im Sommer muessen wir gegensteuern.
        Wir korregieren den Zeitwert mit der Zeitzone des Absenders.
        Der Restfehler ist nun unsere eigene Zeitzone zu GMT.

        Mit localtime bilden wir nun die tm-Struktur neu. Da unser
        Zeitwert aber golobal war ist das Ergebnis um unsere eigene
        Zeitzone verschoben. Das Resultat ist die Zeit des Absenders

        ueber diese Formel werden die Stunden, Wochentage, Datumsübegänge,
        Schaltjahre und sonstige Probleme einfach geloest.

	Dirk Meyer
*/

	t.tm_isdst = 0;
	dt = mktime(&t);
#ifndef	PLUS_OLD_MKTIME
	if ( t.tm_isdst != 0 )
		dt -= 7200L;
#endif
	dt += ( ( ((long)(*tz_hour) * 60L) + (long)(*tz_min)) * 60L );
	t = *localtime( &dt );
	t.tm_isdst = 0;

/* und den Wochentag berechnen... */		
	dt = mktime(&t);

	strftime(answer, 60, "%a, %d %b %Y %H:%M:%S", &t);
	sprintf(buffer, " %+03d%02d", *tz_hour, *tz_min);
	strcat(answer, buffer);

	return answer;
}
