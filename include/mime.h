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
 *   unix-connect-request@mailinglisten.im-netz.de
 *  for instructions on how to join this list.
 */

/*
 * Copyright (C) 1995 M.Both
 * Copyright (C) 1995 C.Creutzig
 */

#ifndef __MIME_H
#define __MIME_H

#define SP_MULTIPART_BOUNDARY  "--8<----"

typedef enum { cte_none, cte_7bit, cte_quoted_printable, cte_base64, cte_8bit,
		cte_binary, cte_x_uuencode, cte_unknown } mime_cte;

typedef enum { cty_none, cty_text, cty_multipart, cty_message, cty_application,
		cty_image, cty_audio, cty_video, cty_unknown } mime_cty;

typedef struct {
	char *name;
	mime_cte cte;
} mime_cte_struct;

typedef struct {
	char *name;
	mime_cty cty;
} mime_cty_struct;

/* Fuer parse_mime_header() fuer Rueckgabewerte */
typedef struct {
	mime_cte encoding;
	mime_cty type;
	int text_plain; /* Type: text/plain */
	int charset; /* undef=0; us-ascii=-1; sonst iso-8859-n */
	char *filename; /* malloc'd, falls vorhanden; sonst NULL */
	int unixconnect_multipart; /* UnixConnect-Spezial-Multipart */
} mime_header_info_struct;

extern mime_cte_struct mime_ctes[];
extern mime_cty_struct mime_ctys[];

#undef ATR_CONST
#ifdef __GNUC__
#define ATR_CONST __attribute__ ((const))
#else
#define ATR_CONST
#endif

int decode_cte(char *msg, long *msglenp, int *eightbit, 
	mime_header_info_struct *info);
int decode_x_uuencode(char *, long *, int *, mime_header_info_struct *);
char *mime_encode(const char *iso) ATR_CONST;
char *mime_address(const char *zcon_ad) ATR_CONST;
int count(const char *s, char c) ATR_CONST;
int is_8_bit(const unsigned char *string) ATR_CONST;
int parse_mime_header(int direction, header_p hd, mime_header_info_struct *);
char *decode_mime_string(const char *buf) ATR_CONST;

#endif /* __MIME_H */
