/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-95  Martin Husemann
 *  Copyright (C) 1995     Christopher Creutzig
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


#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#endif

#include "utility.h"
#include "uulog.h"
#include "crc.h"
#include "header.h"
#include "hd_def.h"
#include "hd_nam.h"

const char *hd_crlf = "\n";

static void
test1(void)
{
	header_p p;

	p = new_header("Test-Inhalt", 2);
	p = del_header(2, p);
	if (p) {
		fprintf(stderr, "FATAL: del_header defekt!\n");
	}

	p = add_header("Test-2-Inhalt", 2, new_header("Test-Inhalt", 2));
	p = del_header(2, p);
	if (p) {
		fprintf(stderr, "FATAL: add_header oder del_header defekt!\n");
	}
}

int
main(int argc, char *argv[])
{
    FILE *f;
    header_p h[100], ges, p, x;
    int i, last = 0;

    if (argc <= 2)
	uufatal(__FILE__, "Aufruf: header.exe (headerdatei) (ausgabedatei)");

    test1();

    for (i=0;i<100;i++) h[i] = NULL;
    f = fopen(argv[1], "rb");
    if (!f) uufatal(__FILE__, "Datei nicht gefunden: %s", argv[1]);
    puts("Lese Daten ....");
    while (!feof(f)) {
         h[last++] = rd_para(f);
    }
    fclose(f);

    f = fopen(argv[2], "wb");
    if (!f) uufatal(__FILE__, "Datei nicht schreibbar: %s", argv[2]);

    puts("Schreibe Daten....");
    for (i=0; i<last; i++)
       if(h[i]) {
       	 h[i] = del_header(HD_CRC, h[i]);
         wr_para(h[i], f);
       }

    fprintf (f, "# Und die Zusammenfassung:%s", hd_crlf);
    ges = h[0];
    for (i=1; i<last; i++)
    	ges = join_header(h[i], ges);
    wr_para(ges, f);

    fprintf (f, "%s%s# Und die X- Header gesondert:%s", hd_crlf, hd_crlf, hd_crlf);
    for (x = ges; x; x = x->next)
    	if ((x->header[0] == 'x' || x->header[0] == 'X') && x->header[1] == '-')
    	   fprintf(f, "%s:\t%s%s", x->header, x->text, hd_crlf);
    fclose(f);

    printf("Teste Hash-Funktion:\n");
    p = find(HD_UU_TO, ges);
    if (!p) {
    	fprintf(stderr, "FATAL: kein " HN_UU_TO ": Header in der Testdatei!\n");
    	return 10;
    } else {
    	if (stricmp(p->header, HN_UU_TO) != 0) {
    		fprintf(stderr, "FATAL: Hashfunktionen stimmen nicht ueberein!\n");
    		return 10;
    	} else {
    		printf("[%s] == [%s]\n", HN_UU_TO, p->header);
       	}
    }
    p = find(HD_UU_FROM, ges);
    if (!p) {
    	fprintf(stderr, "FATAL: kein " HN_UU_FROM ": Header in der Testdatei!\n");
    	return 10;
    } else {
    	if (stricmp(p->header, HN_UU_FROM) != 0) {
    		fprintf(stderr, "FATAL: Hashfunktionen stimmen nicht ueberein!\n");
    		return 10;
    	} else {
    		printf("[%s] == [%s]\n", HN_UU_FROM, p->header);
       	}
    }
    printf("OK\n");

    return 0;
}


