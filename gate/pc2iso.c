/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
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
 * Datei: pc2iso.c  -  wandelt ZCONNECT Zeichen in ISO-8859-1
 *
 * Der ZConnect Zeichensatz ist eine Untermenge des IBM-PC Zeichensatzes,
 * CodePage 437. Erlaubt sind alle Zeichen, die in der folgenden Tabelle
 * nicht als 'BAD' markiert sind.
 *
 * Wir konvertieren mehr Zeichen, als im ZConnect-Zeichensatz spezifiziert
 * sind. Damit wird kein Schaden angerichtet, aber es hilft einigen Leuten.
 */

#include <sys/types.h>

#define BAD 160		/* 160 = 'alternatives' Space im ISO */

/*
 * Table converting IBM Code Page 437 (CP437) to ISO 8859-1
 *
 * Tabelle von Kosta Koastis, Aenderungen von Tetisoft@apg.zer.sub.org
 *
 * 0-31 changed to BAD except Tab,LF,FF,CR
 * 201  changed from '-' to '+'
 * 203  changed from '|' to '-'
 * 237  changed from 'O' to 248
 *
 * changes from Christopher Creutzig, taken from a Zerberus(tm) CHARSET.MBX
 * (I do not see the point in listing more than a dozen changes separately...)
 *
 */

static  unsigned char pc437xiso [] =                    /* 000-255 */
{
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 000-007 */
	BAD,   9,  10, BAD,  12,  13, BAD, BAD,         /* 008-015 */
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 016-023 */
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 024-031 */

	' ', '!',  34, '#', '$',  37, '&',  39,         /* 032-039 */
	'(', ')', '*', '+', ',', '-', '.', '/',         /* 040-047 */
	'0', '1', '2', '3', '4', '5', '6', '7',         /* 048-055 */
	'8', '9', ':', ';', '<', '=', '>', '?',         /* 056-063 */
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',         /* 064-071 */
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',         /* 072-079 */
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',         /* 080-087 */
	'X', 'Y', 'Z', '[',  92, ']', '^', '_',         /* 088-095 */
	 96, 'a', 'b', 'c', 'd', 'e', 'f', 'g',         /* 096-103 */
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',         /* 104-111 */
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',         /* 112-119 */
	'x', 'y', 'z', '{', '|', '}', 126, 127,         /* 120-127 */

	199, 252, 233, 226, 228, 224, 229, 231,         /* 128-135 */
	234, 235, 232, 239, 238, 236, 196, 197,         /* 136-143 */
	201, 230, 198, 244, 246, 242, 251, 249,         /* 144-151 */
	255, 214, 220, 248, 163, 216, 215, 131,         /* 152-159 */
	225, 237, 243, 250, 241, 209, 170, 186,         /* 160-167 */
	191, 174, 172, 189, 188, 161, 171, 187,         /* 168-175 */

	'#', '#', '#', '|', '|', 193, 194, 192,         /* 176-183 */
	169, '|', '|', '+', '+', 162, 165, '+',         /* 184-191 */
	'+', '-', '-', '|', 173, '+', 227, 195,         /* 192-199 */
	'+', '+', '-', '-', '|', '-', '+', 164,         /* 200-207 */
	240, 208, 202, 203, 200, 105, 205, 206,         /* 208-215 */
	207, '+', '+', '#', '#', 166, 204, '#',         /* 216-223 */

	211, 223, 212, 210, 245, 213, 181, 254,         /* 224-231 */
	222, 218, 219, 217, 253, 221, 175, 180,         /* 232-239 */
	173, 177,  95, 190, 182, 167, 247, 184,         /* 240-247 */
	176, 168, 183, 185, 179, 178, 183, ' '          /* 248-255 */
} ;

#include "lib.h"

void pc2iso_size(char *buffer, size_t len)
{
	unsigned char *b;
	unsigned char c;

	b = (unsigned char *)buffer;
	while (len--) {
		c = pc437xiso[*b];
		*b++ = c;
	}
}

void pc2iso(char *buffer)
{
	pc2iso_size(buffer, strlen( buffer ));
}

