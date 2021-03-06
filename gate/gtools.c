/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1996-2000  Dirk Meyer
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
 * gtools.c - Werzeuge
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "istring.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include "idir.h"
#include <errno.h>

#include "gtools.h"
#include "version.h"
#include "sysexits2.h"

char datei[2000];
char baseid[20];

void
do_version( const char *name )
{
	fprintf( stderr,
"%s (Unix-Connect) " VERSION "\n"
"Copyright " COPYRIGHT "\n"
"Unix-Connect comes with ABSOLUTELY NO WARRANTY,\n"
"to the extent permitted by law.\n"
"You may redistribute copies of Unix-Connect\n"
"under the terms of the GNU General Public License.\n"
"For more information about these matters,\n"
"see the files named COPYING.\n"
, name );
	exit( EX_OK );
}

FILE *
open_new_file( const char *name, const char *dir, const char *ext )
{
	time_t j;
	struct stat st;
	int fh;
	int statrc;
	FILE *fp;

	if ( dir == NULL )
		return NULL;

	/* Matthias Andree: Pruefen, ob das Spooldirectory existiert */
	if ((statrc = stat(dir, &st)) || !S_ISDIR(st.st_mode)) {
		if(!statrc) errno=ENOTDIR;
#ifdef ENABLE_AUTO_CREATE
		if (mkdir(dir, 02775) != 0) {
#endif
		fprintf( stderr,
			"%s: error create file in output dir %s: %s\n",
			name, dir, strerror( errno ) );
		exit(EX_CANTCREAT);
#ifdef ENABLE_AUTO_CREATE
		}
#endif
	}
/* Um sicherzugehen, dass wir keine Datei ueberschreiben,
 * oeffnen wir sie zunaechst mit O_EXCL. Um weiter unten
 * einen FILE* zu haben, verwenden wir fdopen */
	errno = 0;
	j = time(NULL);
	do {
		sprintf(baseid, "%08lx", (long)(j++));
		strcpy( datei, dir );
		strcat( datei, baseid );
		if ( ext != NULL )
			strcat( datei, ext );
		fh = open(datei, O_WRONLY|O_CREAT|O_EXCL,
				S_IRUSR|S_IWUSR|S_IRGRP);
		if ( fh >= 0 ) {
			fp = fdopen( fh, "ab" );
			return fp;
		}
	} while ( errno == EEXIST );
	return NULL;
}

