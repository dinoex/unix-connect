/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1999     Dirk Meyer, Matthias Andree
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
 * alias.c
 *
 * Alias-Zugriffsfunktionen
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "utility.h"
#include "lib.h"

#include "alias.h"
#include "utility.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"
#include "config.h"
#include "uulog.h"

extern void minireadstat(void);
static int init_done = 0;
extern char *aliasliste;

int prefix_len;

typedef struct alias_st {
	char *u, *z;
	int verw;
	struct alias_st *next;
} alias_t, *alias_list;

static alias_list data = NULL;

void insert(char *znetz, char *uucp, int v)
{
	alias_list p;

	p = dalloc(sizeof(alias_t));
	p->u = dstrdup(uucp);
	p->z = dstrdup(znetz);
	strlwr(p->u);
	strupr(p->z);
	p->verw = v;
	p->next = data;
	data = p;
}

static void parse_alias(header_p p, int v)
{
	char *m, *n, *o;
	const char *sep = " \t";
	int ok;

	ok=0;
	if((m = dstrdup(p->text))) { /* strtok schiesst im String herum */
		if((n = strtok(m, sep))) {
			if((o = strtok(NULL, sep))) {
				/* sauberer ist das nochmalige Umkopieren */
				insert(dstrdup(n), dstrdup(o), v);
				ok=1;
			}
		}
		free(m);
	}
	if(!ok) {
		fprintf(stderr,
			__FILE__ ":%d ignored malformatted alias line in %s: \"%s\"\n",
			__LINE__,
			aliasliste, p->text);
	}
}

static void init(void)
{
	char *s;

	if (init_done) return;
	init_done = 1;
	data = NULL;
	minireadstat();
	if (aliasliste) {
		FILE *f;
		header_p hd, p;

		f = fopen(aliasliste, "rb");
		if (!f) {
			fprintf(stderr, "cannot open %s: %s\n",
				aliasliste, strerror(errno));
			return;
		}
		hd = NULL;
		while (!feof(f)) {
			p = rd_para(f);
			if (!p) break;
			hd = join_header(hd, p);
		}
		fclose(f);
		for (p=find(HD_ALIAS, hd); p; p=p->other) {
			parse_alias(p, ALIAS);
		}

		for (p=find(HD_MAP, hd); p; p=p->other) {
			parse_alias(p, PREFIX);
		}
	}
}

char *z_search(char *gesucht, int v)
{
	alias_list p;
/* Aus irgendeinem Grund (den ich noch nicht finde konnte)
 * vergißt die Routine manchmal (reproduzierbar) einen Eintrag
 * in der Alias-Liste, sie setzt p->z einfach auf ''.
 * Das gibt natuerlich unerwuenschte Effekte. Symptomkur:
 */
	int error;

	if (!init_done) init();
	do {
		error = 0;
		if (!data) return NULL;
		for (p = data; p; p = p->next) {
			if (strcmp(p->z, "")==0) {
				error = 1;
				init_done=0;
				init();
				break;
			}
			if (v != p->verw) continue;
			if ( v == ALIAS ) {
				if (strcasecmp(p->z, gesucht ) == 0) {
					prefix_len = strlen(gesucht);
					return p->u;
				}
			} else { /* PREFIX */
				if (strncasecmp(p->z, gesucht, strlen( p->z ) ) == 0) {
					prefix_len = strlen(p->z);
					return p->u;
				}
			}
		}
	} while(error != 0);

	prefix_len = 0;
	return NULL;
}

char *u_search(char *gesucht, int v)
{
	alias_list p;

	if (!init_done) init();
	if (!data) return NULL;
	for (p = data; p; p = p->next) {
		if (v != p->verw) continue;
		if ( v == ALIAS ) {
			if (strcasecmp(p->u, gesucht ) == 0) {
				prefix_len = strlen(gesucht);
				return p->z;
			}
		} else {
			if (strncasecmp(p->u, gesucht,strlen(p->u) ) == 0) {
				prefix_len = strlen(p->u);
				return p->z;
			}
		}
	}
	prefix_len = 0;
	return NULL;
}
