/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1995     Moritz Both
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


/* atob: version 4.0
 * stream filter to change printable ascii from "btoa" back into 8 bit bytes
 * if bad chars, or Csums do not match: exit(1) [and NO output]
 *
 *  Paul Rutter		Joe Orost
 *  philabs!per		petsd!joe

Angepasst an UnixConnect von Moritz Both
 */

#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif

#define reg register

#define streq(s0, s1)  strcmp(s0, s1) == 0

#define times85(x)	((((((x<<2)+x)<<2)+x)<<2)+x)

static long int Ceor = 0;
static long int Csum = 0;
static long int Crot = 0;
static long int word = 0;
static long int bcount = 0;
static char *buffer;
static char *bp;

#define DE(c) ((c) - '!')

static int decode(c)
  reg c;
{
  if (c == 'z') {
    if (bcount != 0) {
      return 1;
    } else {
      byteout(0);
      byteout(0);
      byteout(0);
      byteout(0);
    }
  } else if ((c >= '!') && (c < ('!' + 85))) {
    if (bcount == 0) {
      word = DE(c);
      ++bcount;
    } else if (bcount < 4) {
      word = times85(word);
      word += DE(c);
      ++bcount;
    } else {
      word = times85(word) + DE(c);
      byteout((int)((word >> 24) & 255));
      byteout((int)((word >> 16) & 255));
      byteout((int)((word >> 8) & 255));
      byteout((int)(word & 255));
      word = 0;
      bcount = 0;
    }
  } else {
    return 1;
  }
  return 0;
}

byteout(c)
  reg c;
{
  Ceor ^= c;
  Csum += c;
  Csum += 1;
  if ((Crot & 0x80000000)) {
    Crot <<= 1;
    Crot += 1;
  } else {
    Crot <<= 1;
  }
  Crot += c;
  *bp++ = (char)c;
}

long atob(char *buf) {
	long len;
	char *cp;
	long int n1, n2, oeor, osum, orot;

	/* Pruef-Variabelen initialisieren */
	Ceor = 0;
	Csum = 0;
	Crot = 0;
	word = 0;
	bcount = 0;

	len = strlen(buf);
	if (!len)
		return 0;
	buffer = (char *) malloc(len*4+1);
	if (!buffer)
		return 0;
	bp = buffer;
	/*search for header line*/
	cp = strstr(buf, "xbtoa Begin");
	if (!cp) {
		free(buffer);
		return 0;
	}

	for(cp = cp+11; *cp && *cp!='x'; cp++) {
		switch(*cp) {
			case '\r':
			case '\n':
				continue;
			case 'x':
				break;
			default:
				decode(*cp);
		}
	}

	if (!*cp) {
		free(buffer);
		return 0;
	}

  	if (sscanf(cp,"xbtoa End N %ld %lx E %lx S %lx R %lx\n",
         &n1, &n2, &oeor, &osum, &orot) != 5) {
		free(buffer);
		return 0;
	}
	if ((n1 != n2) || (oeor != Ceor) || (osum != Csum) || (orot != Crot)) {
		free(buffer);
		return 0;
	}

	/*OK - gut konvertiert */
	memcpy(buf, buffer,n1);
	free(buffer);
	return (n1) ;
}
