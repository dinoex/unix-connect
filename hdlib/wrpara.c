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


/********************************************************************
 *
 *   File    :  wrpara.c
 *   Autoren :  Martin
 *   Funktion:  Schreibt einen kompletten Header
 *              DO_CRC definiert -> CRC fuer die ZC-Onlinephase
 *                                  wird erzeugt und mitgeschrieben
 */

# include "config.h"
# include <stdio.h>
# include "header.h"
# include "hd_nam.h"

#ifdef DO_CRC
void wr_packet(header_p ptr, FILE *fp)
#else
void wr_para_continue(header_p ptr, FILE *fp)
#endif
{
	crc_t crc;
#ifdef DO_CRC
	char *p;
#endif

	crc = ~0;
        while (ptr) {
                fputs(ptr->header, fp);
#ifdef DO_CRC
                for (p=ptr->header; *p; p++)
                	crc = CRC(*p, crc);
		putc(':', fp);
		crc = CRC(':', crc);
#else
                fputs(":\t", fp);
#endif
                fputs(ptr->text, fp);
#ifdef DO_CRC
                for (p=ptr->text; *p; p++)
                	crc = CRC(*p, crc);
		putc('\r', fp);
#else
                fputs(hd_crlf, fp);
#endif
                if (ptr->other)
                        ptr = ptr->other;
                else
                        ptr = ptr->next;
        }
#ifdef DO_CRC
	fprintf(fp, HN_CRC":%04X\r\r", crc);
	fflush(fp);
#endif
}

