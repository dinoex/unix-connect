/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1996-2000  Dirk Meyer
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

#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <time.h>
#include <stdarg.h>

#include "datelib.h"

typedef struct {
	const char *eda;
	const char *str;
} testeda_t;

const testeda_t Samples[] = {
/* MEST/CEST */
	{ "19960720001008W+2:00",
		"Sat, 20 Jul 1996 02:10:08 +0200" },
/* MET/CET */
	{ "19981231233000W+1:00",
		"Fri, 01 Jan 1999 00:30:00 +0100" },
/* GMT */
	{ "19960721000000W+0:00",
		"Sun, 21 Jul 1996 00:00:00 +0000" },
/* PDT */
	{ "19960524080805W-6:00",
		"Fri, 24 May 1996 02:08:05 -0600" },
/* PST */
	{ "19960224080805W-6:00",
		"Sat, 24 Feb 1996 02:08:05 -0600" },
/* ASIA */
	{ "19960524054239W+8:00",
		"Fri, 24 May 1996 13:42:39 +0800" },
/* Australia */
	{ "19960522141553W+11:00",
		"Thu, 23 May 1996 01:15:53 +1100" },
/* France */
	{ "19960719171700W+1:00",
		"Fri, 19 Jul 1996 18:17:00 +0100" },
	{ NULL, NULL }
};

char answer[ 1024 ];
char sdate[ 1024 ];

static char *
eda2date(const char *text)
{
	struct tm t;
	time_t dt;
	char s_or_w;
	char buffer[24];
	int tz_hour;
	int tz_min;

	tz_hour = tz_min = 0;
	memset(&t, 0, sizeof t);
	sscanf(text, "%04d%02d%02d%02d%02d%02d%c%d:%d",
		&t.tm_year, &t.tm_mon, &t.tm_mday,
		&t.tm_hour, &t.tm_min, &t.tm_sec,
		&s_or_w, &tz_hour, &tz_min);

	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_isdst = -1;

/*
        Ziel: die lokale Zeit des Absenders

        Wir bilden mit mktime einen Zeitwert, als ob es unsere
        eigene lokale Zeit waere. Dies liefert uns einen Fehler
        um die Differez der Zeitzonen von Absender und Gate.
        Wir korregieren den Zeitwert mit der Zeitzone des Absenders.
        Der Restfehler ist nun unsere eigene Zeitzone zu GMT.

        Mit localtime bilden wir nun die tm-Struktur neu. Da unser
        Zeitwert aber golobal war ist das Ergebnis um unsere eigene
        Zeitzone verschoben. Das Resultat ist die Zeit des Absenders

        ueber diese Formel werden die Stunden, Wochentage, Datumsuebegaenge,
        Schaltjahre und sonstige Probleme einfach geloest.

	Dirk Meyer
*/

	dt = mktime(&t);
	dt += ( ( ((long)(tz_hour) * 60L) + (long)(tz_min)) * 60L );
	t = *localtime( &dt );
	t.tm_isdst = -1;

/* und den Wochentag berechnen... */
	dt = mktime(&t);

	strftime(answer, 60, "%a, %d %b %Y %H:%M:%S", &t);
	sprintf(buffer, " %+03d%02d", tz_hour, tz_min);
	strcat(answer, buffer);

	return answer;
}

static char *
date2eda( const char *str )
{
	int tz;
	char *datum;
	struct tm *lt;
	time_t dt;

	datum = strdup(str);
	dt = parsedate(datum, &tz);
	free(datum);
	if (dt == -1) {
		tz = 0;
		dt = time(NULL);
		fputs("U-X-Date-Error: parse error in Date: line\r\n",stderr);
	}
	lt = localtime(&dt);
	sprintf(sdate, "%04d%02d%02d%02d%02d%02dW%+d:%02d",
		lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
		lt->tm_hour, lt->tm_min, lt->tm_sec,
		tz/60, abs(tz%60));
	return sdate;
}

static int
loop_samples( void )
{
	const char *new_date;
	const char *new_eda;
	int i;
	int err;

	err = 0;
	i = 0;
	while( Samples[ i ].eda != NULL ) {
		new_date = eda2date( Samples[ i ].eda );
		if ( strcmp( new_date, Samples[ i ].str ) != 0 ) {
			printf( "Fehler: ZC->RFC\n" );
			printf( " in:%s\n", Samples[ i ].eda );
			printf( "out:%s\n", Samples[ i ].str );
			printf( "new:%s\n", new_date );
			printf( "\n" );
			err ++;
		}
		new_eda = date2eda( Samples[ i ].str );
		if ( strcmp( new_eda, Samples[ i ].eda ) != 0 ) {
			printf( "Fehler: RFC->ZC\n" );
			printf( " in:%s\n", Samples[ i ].str );
			printf( "out:%s\n", Samples[ i ].eda );
			printf( "new:%s\n", new_eda );
			printf( "\n" );
			err ++;
		}
		i ++;
	}
	return err;
}

int
main( void )
{
	int err;

	err = 0;
	err += loop_samples();
	if ( err == 0 )
		exit( 0 );

	printf( "%d Errors found\n", err );
	exit( 1 );
	return 0; /* gcc hat ein Problem */
}

