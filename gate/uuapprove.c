/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1998  Christopher Creutzig
 *  Copyright (C) 1999-2000  Dirk Meyer
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
 *   approve.c: Liste der Newsgroups mit Approved: Header verwalten.
 */


#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <ctype.h>

#include "utility.h"
#include "uulog.h"
#include "uuapprove.h"

extern char *approvedliste;

typedef struct list_st {
	char *brett;
	struct list_st *next;
} *list_p, list_t;

static list_p groups = NULL;

void
init_approved(void)
{
	FILE *f;
	char buffer[500], *p;
	list_p neu;

	if (!approvedliste) return;
	f = fopen(approvedliste, "r");
	if (!f) return;
	while (!feof(f)) {
		if (!fgets(buffer, 500, f)) break;
		for (p=buffer; *p && isspace(*p); p++)
			;
		if (*p == '#')
			continue;
		if (p != buffer)
			strcpy(buffer, p);
		for (p=buffer; *p; p++)
			if (isspace(*p)) break;
		*p = '\0';
		neu = malloc(sizeof(list_t));
		if (!neu) out_of_memory(__FILE__);
		neu->next = groups;
		neu->brett = strdup(buffer);
		if (!(neu->brett)) out_of_memory(__FILE__);
		groups = neu;
	}
	fclose(f);
}

int
approved(const char *newsgroup)
{
	list_p p;

	if (!groups) return 0;

	for (p=groups; p; p=p->next) {
		if (stricmp(p->brett, newsgroup) == 0)
			return 1;
	}
	return 0;
}
