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
		if (!f) return;
		hd = NULL;
		while (!feof(f)) {
			p = rd_para(f);
			if (!p) break;
			hd = join_header(hd, p);
		}
		fclose(f);
		for (p=find(HD_ALIAS, hd); p; p=p->other) {
			for (s=p->text; *s; s++)
				if (isspace(*s)) break;
			if (!*s) continue;
			*s = '\0';
			for (s++ ; *s && isspace(*s); s++)
				;
			insert(p->text, s, ALIAS);
		}
		for (p=find(HD_MAP, hd); p; p=p->other) {
			for (s=p->text; *s; s++)
				if (isspace(*s)) break;
			if (!*s) continue;
			*s = '\0';
			for (s++ ; *s && isspace(*s); s++)
				;
			insert(p->text, s, PREFIX);
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
