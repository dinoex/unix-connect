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


/********************************************************************
 *
 *   File    :  header.h
 *   Autoren :  Hacko, Martin
 *   Funktion:  Definition aller Zugriffsfunktionen und Datentypen fuer
 *              Mail-Header
 */

#ifndef SYSDEP_H
#include "sysdep.h"
#endif


# ifndef _HEADER
# define _HEADER

#include "crc.h"

/*
 * Definition des Header-Records fuer Mail-Header
 */

typedef struct para_s {
	char *text;      /* Pointer auf den Inhalts-String */
	char *header;    /* Pointer auf den Namen dieses Headers */
	unsigned code;   /* Interner Code dieses Header-Namens */

        struct para_s
             *other,     /* Pointer auf weitere Header mit diesem code */
             *next;      /* Pointer auf den n„chsten Header */
} header_t;

typedef header_t *header_p;

/* Fehlercodes fr rd_para */
#define HEAD_NO_ERROR 0
#define HEAD_END_OF_FILE 1
#define HEAD_ILLEGAL_HEADER 2

extern int rd_para_error;

#define MAX_HEADER_NAME_LEN	1024

#define HD_UU_CONTROL HD_CONTROL
#define HN_UU_CONTROL HN_CONTROL

/*
 * Die Funktionen:
 */

extern char *hd_crlf;	/* Muss vom main() definiert werden, um fuer dieses */
			/* Programm die Zeilenendekonvention festzulegen */

/* void wr_para (header_p, FILE *); */
#define wr_para(p,f) { wr_para_continue(p,f); fputs(hd_crlf, f); }

int set_rd_para_reaction (int new_reaction);
header_p uu_rd_para(FILE *);
header_p smtp_rd_para(FILE *);
header_p rd_para(FILE *);

/*
 * Versionen mit CRC Behandlung: (und anderen Zeilenenden)
 */
header_p rd_packet(FILE *, crc_t *);	/* CRC ist der lokal ermittelte */
void wr_packet(header_p, FILE *);	/* Der CRC wird automatisch hinzugef. */

header_p rd_para_str(char *str);
char *wr_para_str(header_p ptr, char *buffer,int size);
void     wr_para_continue(header_p, FILE *);
void     wr_paraf(header_p ptr, FILE *fp,int laenge);
long     size_para(header_p);
void     do_free_para(header_p);
header_p find(unsigned, header_p);
header_p truefind(int, header_p);
char *	 header_name(int);
char *headertext (header_p, int, char *);
header_p internal_add_header(char *, int, header_p, const char *);
header_p add_header(char *, int, header_p);
header_p replace_header(char *, int, header_p);
header_p del_header(unsigned, header_p);
header_p join_header(header_p, header_p);
header_p unify_header(header_p, header_p);
#define copy_header(p) join_header(p,NULL)
unsigned identify(const char *);

/* header_p new_header(char *, int); */
# define new_header(text, code) add_header(text, code, NULL)
# define for_header(par, code, h) for(par=find(code, h); par; par=par->other)
#endif
