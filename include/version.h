/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1997-2000  Matthias Andree
 *  Copyright (C) 1995-2000  Dirk Meyer
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
 * version.h
 */

#define	VERSION		"0.94"

#define	MAILER		"UNIX/Connect"
#define	COPYRIGHT	\
"(C) 1993-1994 Martin Husemann,\n" \
"(C) 1995-1998 Christopher Creutzig, (C) 1995-2000 Moritz Both,\n" \
"(C) 1995-2001 Dirk Meyer, (C) 1997-2001 Detlef Wuerkner,\n" \
"(C) 1997-2000 Matthias Andree, (C) 1997-2000 Andreas Barth,\n" \
"(C) 1999-2000 Krischan Jodies.\n"

/*
 *   Fuer die Online-Phase:
 */
#define PROGRAM		MAILER " fuer " BRAND

