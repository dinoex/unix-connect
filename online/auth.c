/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995-1996  Christopher Creutzig
 *  Copyright (C) 1999       Dirk Meyer
 *  Copyright (C) 1999-2000  Andreas Barth
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
 * Authentifikationsroutinen, derzeit nur fuer januslogin
 * komplett ueberarbeitet von Andreas Barth 4. Oktober 1999
 *
 */

#include "auth.h"
#include "zconnect.h"
#include "locknames.h"
#include "utility.h"

#include "idir.h"
#include "istring.h"

static header_p
readsysfile(const char *sysdir, const char *sysname)
{
	DIR *dir;
	struct dirent *this;
	char buf[FILENAME_MAX], *p;
	char filename[FILENAME_MAX];
	FILE *f;
	header_p hd;

	if ((dir = opendir(sysdir)) == NULL) {
		newlog(ERRLOG, "sysdir %s nicht gefunden", sysdir);
		return NULL;
	}
	while ((this = readdir(dir)) != NULL) {
		stccpy(buf, this->d_name, FILENAME_MAX);
		p = strchr(buf, '.');
		if (p) *p = '\0';
		strlwr(buf);
		if (stricmp(buf, sysname) == 0) {
			break;
		}
	}
	closedir(dir);

	if (stricmp(buf,sysname) == 0) {
		stccpy(buf, this->d_name, FILENAME_MAX);

		snprintf(filename, sizeof(filename)-1,
			"%s/%s", sysdir, this->d_name);

		f = fopen(filename, "rb");
		if (!f) {
			return (NULL);
		}
		hd = rd_para(f);
		fclose(f);

		return (hd);
	}

	return NULL;
}


static char*
lwrdup(const header_p hd, const int hzahl, const char *hname)
{
	char *x;
	header_p p;

        if (!(p = find(hzahl, hd))) {
                newlog(ERRLOG, "Header %s nicht vorhanden", hname);
                return NULL;
        }
        if (!(x = strdup(p->text))) {
                newlog(ERRLOG, "malloc schlaegt fehl");
                return NULL;
        }
        strlwr(x);
	return x;
}



auth_t*
authsys(const int version, const char *sysname, const char *passwd, const char *sysdir) {

/*
 * Liest ein Sysfile ein, verifiziert das Passwort und gibt
 * den arcer, arcerin und transfer zurueck.
 *
 * Bugs: Der "Header" hd wird nicht wieder freigegeben.
 *
 * Rueckgabewert: auth_t, wenn ok. Sonst NULL
 *
 * ABa, 6. Oktober 1999
 */

	auth_t *auth;
	header_p hd,p;
	char *x;

	if (version > 1) {
		return NULL;
	}

	if (!(hd=readsysfile(sysdir, sysname))) {
		return NULL;
	}


	/* Stimmt das Passwort */
	if (!(p = find(HD_PASSWD, hd)) || stricmp(p->text, passwd) != 0) {
		return NULL;
	}


	/* und die Strukture besorgen */
	if (!(auth = (auth_t *)malloc(sizeof(auth_t)))) {
                newlog(ERRLOG, "malloc schlaegt fehl");
		return NULL;
	}

	if (!(
	(auth->ssysname = lwrdup(hd, HD_SYS, HN_SYS)) &&
	(auth->arcerout = lwrdup(hd, HD_ARCEROUT, HN_ARCEROUT)) &&
	(auth->arcerin  = lwrdup(hd, HD_ARCERIN, HN_ARCERIN)) &&
	(auth->transfer = lwrdup(hd, HD_PROTO, HN_PROTO))
	)) {
		return NULL;
	}

	if (!(p = find(HD_DOMAIN, hd))) {
		return NULL;
	}
        if (!(x = (char *) malloc(
	   (strlen(p->text)+strlen(auth->ssysname)+2)*(sizeof(char))))
            ) {
                newlog(ERRLOG, "malloc schlaegt fehl");
                return NULL;
        }
	sprintf (x, "%s.%s", auth->ssysname, p->text);
	auth->lsysname=x;

	return auth;
}
