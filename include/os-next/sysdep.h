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
 *  write a mail with subject "Help" to
 *   nora.e@mailinglisten.im-netz.de
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
#define	BRAND	"NeXTSTEP"

#define BSD	/* beliebiges BSD, auch z.B. NeXT */
#define NEXTSTEP

#define HAS_STRINGS_H /* Die str... Funktionen sind in <strings.h> */

#define HAS_BSD_SGTTY	/* benutze <sgtty.h> TTY-Interface */

#define	HAS_BSD_DIRECT	/* struct direct statt struct dirent */

#define HAS_BSD_GETWD	/* getwd() mit einem Argument */
#define NO_UNISTD_H	/* Kein <unistd.h> */
#define HAS_SYS_FCNTL_H	/* #include <sys/fcntl.h> */

/*
 *  Die Online-Empfangsprogramme koennen ohne Passwortabfrage des
 *  Systems eingebunden werden, sie fragen daher selbst nach einem
 *  Passwort:
 */

#define	ASK_PASSWD


/*
 *  Fehlende Prototypen in System-Headern:
 */
 
extern char *strdup(const char *);

#define MAXINT 0x7fffffff

#define	FILENAME_MAX	1024
#define	SEEK_SET	0

#define pid_t int
#define WEXITSTATUS(x)	(x.w_retcode)

#endif
