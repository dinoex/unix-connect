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


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif

#include "lib.h"

#define	WAIT_SLEEP	5

/*
 *  Prüfe ein lock-File und warte gegebenenfalls, bis es verschwindet.
 *  'timeout' gibt die Warte-Strategie an:
 *
 *	< 0 :	warte maximal abs(timeout) Sekunden und versuch dann, den
 *		Lock zu entfernen
 *	  0 :	warte zur Not ewig
 *	> 0 :	warte maximal timeout Sekunden und gib dann auf
 *
 *   Returns:	0 bei Erfolg
 */
int waitnolock(const char *fname, long timeout)
{
	long tout; time_t start, now;

	tout = timeout < 0 ? -timeout : timeout;
	start = time(NULL);

	while (access(fname, R_OK) == 0) {
		sleep(WAIT_SLEEP);
		if (timeout) {
			now = time(NULL);
			if ((now-start) >= tout) {
				if (timeout > 0)
					return -1;
				remove(fname);
			}
		}
	}

	return (access(fname, R_OK) == 0);
}

