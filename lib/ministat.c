/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
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


/*
 *  ministat.c
 *
 *  Liest die Gateway-Konfigurationsdatei und stellt entsprechende
 *  Variablen zur Verfuegung.
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 21-Feb-1993  KK  Dokumentation erstellt.
 * 25-Feb-1993  MH  Dokumentation ergaenzt, Defaults korrigiert
 * 09-Apr-1993	MH  Routing-Eintraege ergaenzt
 * 22-Feb-1994	MH  Auf aktuellen Config-Stand angepasst
 * 23-Nov-1995  CC  ZNETZ-Unterstuetzung entfernt
 * 10-Oct-1996  CC  Dirk Meyers automatische Entscheidung Sommer/Winterzeit
 *
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
#include <time.h>

#include "utility.h"
#include "boxstat.h"
#include "crc.h"
#include "header.h"
#include "hd_nam.h"
#include "hd_def.h"
#include "uulog.h"
#include "ministat.h"

/* Globale Variable, siehe boxstat.h enthaelt:
 *
 * - System-Name:
 * - System-Domain:
 * - Zeitzone:
 * - Sommerzeit:
 *
 */

bs_st boxstat;

/*
 * Nach den entsprechenden Gate-X-Y: Zeilen kann auch weiter unten gesucht
 * werden, damit man sich besser im Source zurecht findet.
 */
char *systemedir;       /* Systeme-Dir:		*/
char *backoutdir;	/* Backout-Dir:		*/
char *backindir;	/* BackIn-Dir:		*/
char *fileserverdir;    /* Fileserver-Dir:	*/
char *fileserveruploads;/* Fileserver-Upload-Dir:*/
char *einpack;          /* Arc-Kommando:	*/
char *auspack;          /* Xarc-Kommando:	*/
char *import_prog;      /* Netcall-Einleser	*/
char *logdir;           /* Logbuch-Dir:         */
char *lockdir;          /* Lock-Dir:            */
char *myself;           /* Who-Am-I:		*/
char *autoeintrag;      /* Autoeintrag-Defaults:*/
char *aliasliste;       /* Gate-Alias-Liste:    */
char *approvedliste;    /* Gate-Approved-Liste: */
char *netcalldir;       /* Netcall-Dir:         */
char *path_qualifier;	/* Path-Qualifier:	*/
char *default_path;	/* Path-Default:	*/
int  debuglevel;	/* Debuglevel:		*/
char *inewscmd;         /* Inews incl. Parameter*/
char *ortsnetz;		/* Lokale Vorwahl 	*/
char *fernwahl;		/* vor einer Fernzone-Nummer (0) */
char *international;	/* vor einer internationalen Nummer (00) */

list_t local_domains;	/* Liste der lokal erreichbaren Domains */

header_p config;

/* Guard zur Verhinderung von Doppel-Initialisierungen */
static int init_done = 0;

/*@@
 *
 * NAME
 *   minireadstat -- Liest die Konfigurationsdatei des Gateways und
 *                   stellt entsprechende globale Variablen zur Verfuegung.
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   void minireadstat(void);
 *
 * DESCRIPTION
 *   Die Funktion liest die Konfigurationsdatei des Gateways ein und
 *   setzt die zu jeder Konfigurationszeile gehoerende Variable auf
 *   einen passenden Wert.
 *
 * PARAMETER
 *   keine.
 *
 * RESULT
 *   keines.
 *
 *   Als Seiteneffekt wird die externe Variable boxstat initialisiert.
 *
 * BUGS
 *   Bei Zeilen, die in der Konfiguration fehlen, wird entweder NULL
 *   eingetragen oder mit einer Meldung abgebrochen. Die Behandlung
 *   sollte eindeutig sein und es sollten *sichere* Defaults genommen
 *   werden, wenn ueberhaupt ein Default genommen wird.
 *
 *   Die Namen der Konfigurationszeilen sollten einheitlich sein. Derzeit
 *   ist es ein wilder Mix zwischen Zeilen mit "Gate-"-Prefix und Zeilen
 *   ohne Prefix. Es sollte verschiedene Klassen von Prefixen fuer
 *   die verschiedenen Abschnitte/log. Subsysteme des Gateways geben.
 *
 */

void minireadstat(void)
{
  /*
   *   Die Konfigurationsdatei wird als ein grosser Header einer
   *   Nachricht betrachtet. Auf diese Weise koennen die Routinen
   *   zum Lesen von Headerdateien auch zum Lesen der Konfigdatei
   *   benutzt werden.
   */
  header_p p;
  FILE *f;

  /* Guard gegen Doppel-Aufruf */
  if (init_done)
    return;
  init_done = 1;

  /* Oeffnen der Konfigurationsdatei */
  logdir = dstrdup( "." );
  f = fopen(CONFIG_FILE, "r");
  if (!f)
    uufatal("Konfiguration",
	    "Kann CONFIG Datei \""CONFIG_FILE"\" nicht lesen");

  config = NULL;
  /* Einlesen und Abspeichern des Config-"headers" */
  while (!feof(f)) {
    p = rd_para(f);
    if (!p) break;
    config = join_header(p, config);
  }
  fclose(f);

  /* Jetzt suche nach den betreffenden Konfigurations-
   * parametern und Eintragen derselben in die passenden
   * Variablen.
   *
   * Fehlt der entsprechende Header, so wird ein Default-
   * Wert eingetragen.
   *
   */

  /* Gate-Alias-Liste: */
  p = find(HD_GATE_ALIAS_LISTE, config);
  if (p)
    aliasliste = dstrdup(p->text);
  else
    aliasliste = NULL;		/* Aliasing ist nicht aktiv */

  /* Gate-Approved-Liste: */
  p = find(HD_GATE_APPROVED_LISTE, config);
  if (p)
    approvedliste = dstrdup(p->text);
  else
    approvedliste = NULL;	/* Keine Approved: Header erzeugen */

  /* Systeme-Dir: */
  p = find(HD_SYSTEME_DIR, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_SYSTEME_DIR" Eintrag in der CONFIG-Datei");

  systemedir = dstrdup(p->text);

  /* Logbuch-Dir: */
  p = find(HD_LOGBUCH_DIR, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_LOGBUCH_DIR" Eintrag in der CONFIG-Datei");

  logdir = dstrdup(p->text);

  /* Lock-Dir: */
  p = find(HD_LOCK_DIR, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_LOCK_DIR" Eintrag in der CONFIG-Datei");

  dfree( lockdir );
  lockdir = dstrdup(p->text);

  /* Netcall-Dir: */
  p = find(HD_NETCALL_DIR, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_NETCALL_DIR" Eintrag in der CONFIG-Datei");

  netcalldir = dstrdup(p->text);

  /* Fileserver-Dir: */
  fileserverdir = NULL;
  p = find(HD_FILESERVER_DIR, config);
  if (p)
    fileserverdir = dstrdup(p->text);

  /* Backout-Dir: */
  backoutdir = NULL;
  p = find(HD_BACKOUT_DIR, config);
  if (p)
    backoutdir = dstrdup(p->text);

  /* Backin-Dir: */
  backindir = NULL;
  p = find(HD_BACKIN_DIR, config);
  if (p)
    backindir = dstrdup(p->text);

  /* Fileserver-Upload-Dir: */
  fileserveruploads = NULL;
  p = find(HD_FILESERVER_UPLOAD_DIR, config);
  if (p)
    fileserveruploads = dstrdup(p->text);

  /* Arc-Kommando: */
  p = find(HD_ARC_KOMMANDO, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_ARC_KOMMANDO" Eintrag in der CONFIG-Datei");

  einpack = dstrdup(p->text);

  /* Xarc-Kommando: */
  p = find(HD_XARC_KOMMANDO, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_XARC_KOMMANDO" Eintrag in der CONFIG-Datei");

  auspack = dstrdup(p->text);

  /* Import-Kommando: */
  p = find(HD_IMPORT_KOMMANDO, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_IMPORT_KOMMANDO" Eintrag in der CONFIG-Datei");

  import_prog = dstrdup(p->text);

  /* Who-Am-I: */
  p = find(HD_WHO_AM_I, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_WHO_AM_I" Eintrag in der CONFIG-Datei");

  myself = dstrdup(p->text);

  /* Autoeintrag-Defaults: */
  autoeintrag = NULL;
  p = find(HD_AUTOEINTRAG_DEFAULTS, config);
  if (p)
    autoeintrag = dstrdup(p->text);

  /* Debuglevel: */
  debuglevel = 0;
  p = find(HD_DEBUGLEVEL, config);
  if (p)
    debuglevel = atoi(p->text);

  /* Ortsnetz: */
  ortsnetz = NULL;
  p = find(HD_TEL_ORTSNETZ, config);
  if (p)
    ortsnetz = dstrdup(p->text);

  /* Fernwahl: */
  fernwahl = NULL;
  p = find(HD_TEL_FERNWAHL, config);
  if (p)
    fernwahl = dstrdup(p->text);

  /* International: */
  international = NULL;
  p = find(HD_TEL_INTERNATIONAL, config);
  if (p)
    international = dstrdup(p->text);


  /*
   *  Liste der lokalen Domains:
   */
  local_domains = NULL;
  for (p = find(HD_LOCAL_DOMAIN, config); p; p = p->other) {
    list_t new;

    strlwr(p->text);
    new = dalloc(sizeof(node_t));
    new->next = local_domains;
    new->text = dstrdup(p->text);
    local_domains = new;
  }

  p = find(HD_PATH_DEFAULT, config);
  if (p)
    default_path = dstrdup(p->text);
  else
    default_path = dstrdup("%s");
  p = find(HD_PATH_QUALIFIER, config);
  if (p)
    path_qualifier = dstrdup(p->text);
  else
    path_qualifier = NULL;

  /* System-Name: */
  p = find(HD_SYSTEM_NAME, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_SYSTEM_NAME" Eintrag in der CONFIG-Datei\n");
  boxstat.boxname = dstrdup(p->text);

  /* Gate-Domain: */
  p = find(HD_GATE_DOMAIN, config);
  if (!p || strlen(p->text)==0)
    uufatal("Konfiguration",
	    "kein "HN_GATE_DOMAIN" Eintrag in der CONFIG-Datei\n");
  boxstat.boxdomain = dstrdup(p->text);
}
