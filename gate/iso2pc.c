/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-96  Christopher Creutzig
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
 *   Datei: iso2pc.c  -  konvertiert die im ZCONNECT darstellbare Untermenge
 *                       von ISO-8859-1 ins ZCONNECT Format
 *
 * Der ZConnect Zeichensatz ist eine Untermenge des IBM-PC Zeichensatzes,
 * CodePage 437.
 * Die Konvertierung von ISO 8859-1 (z.B. bei RFC-Relays)
 * geschieht nach den folgenden Tabellen. Alle mit 'BAD'
 * markierten Zeichen sind verboten.
 */

#include <sys/types.h>
 
#define BAD 255		/* 255 = 'alternatives' Space im PC 437 Zeichensatz */

/*
 * Table converting ISO 8859-1 to IBM Code Page 437 (CP437)
 *
 * 0-31  changed to BAD except Tab,LF,FF,CR
 * 173   changed from '-' to 196
 * 248   changed from 'o' to 237
 *
 * Changes by Christopher, ripped from a Zerberus(tm) CHARSET.MBX
 * (I do not see the point in listing more than a dozen changes separately...)
 *
 */

static unsigned char isoxpc437 [] =                     /* 000-255 */
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
	'X', 'Y', 'Z', '[',  92, ']', '^',  95,         /* 088-095 */
	 96, 'a', 'b', 'c', 'd', 'e', 'f', 'g',         /* 096-103 */
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',         /* 104-111 */
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',         /* 112-119 */
	'x', 'y', 'z', '{', '|', '}', 126, 127,         /* 120-127 */

	BAD, BAD, BAD, 159, BAD, BAD, BAD, BAD,         /* 128-135 */
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 136-143 */
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 144-151 */
	BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,         /* 152-159 */

	' ', 173, 189, 156, 207, 190, 221, 245,         /* 160-167 */
	249, 184, 166, 174, 170, 240, 169, 238,         /* 168-175 */
	248, 241, 253, 252, 239, 230, 244, 250,         /* 176-183 */
	247, 251, 167, 175, 172, 171, 243, 168,         /* 184-191 */
	183, 181, 182, 199, 142, 143, 146, 128,         /* 192-199 */
	212, 144, 210, 211, 222, 214, 215, 216,         /* 200-207 */
	209, 165, 227, 224, 226, 229, 153, 'x',         /* 208-215 */
	157, 235, 233, 234, 154, 237, 232, 225,         /* 216-223 */
	133, 160, 131, 198, 132, 134, 145, 135,         /* 224-231 */
	138, 130, 136, 137, 141, 161, 140, 139,         /* 232-239 */
	208, 164, 149, 162, 147, 228, 148, 246,         /* 240-247 */
	155, 151, 163, 150, 129, 236, 231, 152          /* 248-255 */
} ;

void iso2pc(unsigned char *buffer, size_t len)
{
	unsigned char c;

	while (len--) {
		c = isoxpc437[*buffer];
		*buffer++ = c;
	}
}
