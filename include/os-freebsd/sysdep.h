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
 *  etc. are welcome. Contact the author by E-Mail: christopher@nescio.zerberus.de
 *  or snail-mail: Christopher Creutzig, Josefstr. 18, 33106 Paderborn, FRG.
 *
 *  There is a mailing-list for user-support: unix-connect@owl.de,
 *  write a mail with subject "Help" to majordomo@owl.de for instructions
 *  on how to join this list.
 */


/*
 *  UNIX-Connect: sysdep.h
 *
 *  Systemabhaengige C Anweisungen.
 *
 *  Alle fuer dieses System gueltigen #define's sollten hier gesetzt
 *  werden, alle nicht von System-Headern bereitgestellten Prototypen
 *  gehoeren hier hin.
 *
 */

#ifndef SYSDEP_H
#define SYSDEP_H

#define	OS	"UNIX"
#define	BRAND	"BSDI BSD/386"

#ifndef BSD	/* param.h */
#define BSD	/* beliebiges BSD, auch z.B. NeXT */
#endif

#define HAS_STRING_H /* Die str... Funktionen sind in <string.h> */
#define HAS_SYS_FCNTL_H
#define HAS_UNISTD_H

#define HAS_POSIX_TERMIOS
#define NEED_CRTSCTS		/* brauche HW-Handshake in termios */

#define LEAVE_CTRL_TTY

/* man weiss nie... */
#include <limits.h>
#define MAXINT INT_MAX

/*
 *  Die Online-Empfangsprogramme koennen ohne Passwortabfrage des
 *  Systems eingebunden werden, sie fragen daher selbst nach einem
 *  Passwort:
 */

#define	ASK_PASSWD


#endif
