/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-94  Martin Husemann
 *  Copyright (C) 1995-98  Christopher Creutzig
 *  Copyright (C) 1994     Peter Much
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
 *  uulog.h
 *
 *  Logfile-Routinen fuer den ZCONNECT/RFC GateWay
 *
 */

#define Z2ULOG		1
#define U2ZLOG		2
#define INCOMING	4
#define ERRLOG		3
#define OUTGOING	5
#define XTRACTLOG	6
#define DEBUGLOG	7

#ifdef HAVE_SYSLOG
#define SYSLOG_KANAL	LOG_LOCAL0
#define SYSLOG_LOGNAME	"uconnect"
#define FATALLOG_PRIO	LOG_CRIT

#define Z2ULOG_NAME	"zc2mail"
#define Z2ULOG_PRIO	LOG_INFO

#define U2ZLOG_NAME	"mail2zc"
#define U2ZLOG_PRIO	LOG_INFO

#define ERRLOG_NAME	"error"
#define ERRLOG_PRIO	LOG_ERR

#define INCOMING_NAME	"caller"
#define INCOMING_PRIO	LOG_NOTICE

#define OUTGOING_NAME	"calling"
#define OUTGOING_PRIO	LOG_INFO

#define XTRACTLOG_NAME	"extract"
#define XTRACTLOG_PRIO	LOG_INFO

#define DEBUGLOG_NAME	"debug"
#define DEBUGLOG_PRIO	LOG_DEBUG

#endif

#define Z2ULOG_FILE		"import.log"
#define U2ZLOG_FILE		"export.log"
#define ERRLOG_FILE		"errors.log"
#define ERRLOG_FILE		"errors.log"
#define INCOMING_FILE		"anrufe.log"
#define OUTGOING_FILE		"telefon.log"
#define XTRACTLOG_FILE		"extract.log"
#define DEBUGLOG_FILE		"debug.log"

extern const char *nomem;

void initlog(const char *name);

void newlog(int lchan, const char *format, ...)
#ifdef __GNUC__
__attribute__ ((format(printf,2,3)))
#endif
; 

void uufatal(const char *prog, const char *format, ...)
#ifdef __GNUC__
__attribute__ ((noreturn, format(printf,2,3)))
#endif
;

#define out_of_memory(prg)	uufatal(prg, nomem)

