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
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
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
#define	BRAND	"System V"

#define SYSV	/* beliebiges System V */

#include <sys/types.h>	/* ISC Typedefs */

#ifndef FILENAME_MAX
#define FILENAME_MAX 1024	/* POSIX verlangt das in <stdio.h> */
#endif

#define HAS_POSIX_TERMIOS	/* benutze <termios.h> IOCTL's */

#endif
