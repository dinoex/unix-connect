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
 *  uulog.h
 *
 *  Logfile-Routinen fÅr den ZCONNECT/RFC GateWay
 *
 *  Sat Jul  1 20:45:39 MET DST 1995 (P.Much)
 *  - Erweitert zur Nutzung der syslog-facility mit -DLOGSYSLOG.
 */

#ifndef SYSDEP_H
#include "sysdep.h"
#endif


#ifdef LOGSYSLOG
#define SYSLOG_KANAL	LOG_MAIL
#define SYSLOG_LOGNAME	"uconnect"
#define FATALLOG_PRIO	LOG_CRIT

# define Z2ULOG		1
# define Z2ULOG_NAME	"zc2mail"
# define Z2ULOG_PRIO	LOG_INFO
# define U2ZLOG		2
# define U2ZLOG_NAME	"mail2zc"
# define U2ZLOG_PRIO	LOG_INFO
# define ERRLOG		3
# define ERRLOG_NAME	"error"
# define ERRLOG_PRIO	LOG_ERR
# define INCOMING	4
# define INCOMING_NAME	"caller"
# define INCOMING_PRIO	LOG_NOTICE
# define OUTGOING	5
# define OUTGOING_NAME	"calling"
# define OUTGOING_PRIO	LOG_INFO
# define XTRACTLOG	6
# define XTRACTLOG_NAME	"extract"
# define XTRACTLOG_PRIO	LOG_INFO
# define DEBUGLOG	7
# define DEBUGLOG_NAME	"debug"
# define DEBUGLOG_PRIO	LOG_DEBUG
#else
# define Z2ULOG		"import.log"
# define U2ZLOG		"export.log"
# define ERRLOG		"errors.log"
# define INCOMING	"anrufe.log"
# define OUTGOING	"telefon.log"
# define XTRACTLOG	"extract.log"
# define DEBUGLOG	"debug.log"
#endif

extern char *nomem;


#ifdef LOGSYSLOG
void logfile(int lchan, char *from, char *to, char *mid, char *format,...)
#else
void logfile(char *lfilename, char *from, char *to, char *mid, char *format,...)
#endif
#ifdef __GNUC__
__attribute__ ((format(printf,5,6)))
#endif
;

void uufatal(char *prog, char *format, ...)
#ifdef __GNUC__
__attribute__ ((noreturn, format(printf,2,3)))
#endif
;

#define out_of_memory(prg)	uufatal(prg, nomem)
#define out_of_mem(prg,line)	uufatal(prg, "Out of memory in line %d", line)
