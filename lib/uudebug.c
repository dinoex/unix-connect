/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1999     Matthias Andree
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
 *  uudebug.c
 *
 *  Logfile-Routinen fuer den ZCONNECT/RFC GateWay
 *
 */


#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif

#include "uulog.h"
#include "uudebug.h"

void
logcwd(const char *func)
{
#ifndef NDEBUG
	char cwd[1024];
	if(!getcwd(cwd, sizeof(cwd))) {
		newlog(ERRLOG, "cannot get cwd for function %s: %s", func,
		       strerror(errno));
	} else {
		newlog(DEBUGLOG, "cwd for function %s: %s", func, cwd);
	}
#endif
}
