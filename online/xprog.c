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


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef HAS_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifndef NO_UNISTD_H
#include <unistd.h>
#endif
#include "boxstat.h"
#include "ministat.h"
#ifdef HAS_STRING_H
# include <string.h>
#endif
#ifdef HAS_STRINGS_H
# include <strings.h>
#endif
#include "lib.h"
#include "xprog.h"

int recvfile(const char *proto, const char *file)
{
	char cmd[20], arg[20];
	pid_t c_pid;

	if (stricmp(proto, "ZMODEM") == 0) {
		strcpy(cmd, "rz");
		arg[0] = '\0';
	} else	if (stricmp(proto, "YMODEM") == 0) {
		strcpy(cmd, "rb");
		arg[0] = '\0';
	} else	if (stricmp(proto, "XMODEM") == 0) {
		strcpy(cmd, "rx");
		strcpy(arg, file);
	}

	if ((c_pid = fork()) == 0) {
		/* Ich bin child */
		if (arg[0])
			execlp(cmd, cmd, "-v", arg, NULL);
		else
			execlp(cmd, cmd, "-v", NULL);
		perror(cmd);
		return 1;
	} else {
#if defined (NEXTSTEP)
		union wait stat;
#else
		int stat;
#endif
		wait(&stat);

		if (WIFEXITED(stat))
		{
#ifdef DIRTY_ZMODEM_HACK
			if (WEXITSTATUS(stat)==128)
				return 0;
#endif
			return WEXITSTATUS(stat);
		}
		else
			return 0;
	}
}

int sendfile(const char *proto, const char *file)
{
	char cmd[20], arg[20];
	pid_t c_pid;

	if (stricmp(proto, "ZMODEM") == 0) {
		strcpy(cmd, "sz");
		strcpy(arg, file);
	} else	if (stricmp(proto, "YMODEM") == 0) {
		strcpy(cmd, "sb");
		strcpy(arg, file);
	} else	if (stricmp(proto, "XMODEM") == 0) {
		strcpy(cmd, "sx");
		strcpy(arg, file);
	}

	if ((c_pid = fork()) == 0) {
		/* Ich bin child */
		execlp(cmd, cmd, "-vv", arg, NULL);
		perror(cmd);
		return 1;
	} else {
#if defined (NEXTSTEP)
		union wait stat;
#else
		int stat;
#endif
		wait(&stat);

		if (WIFEXITED(stat))
			return WEXITSTATUS(stat);
		else
			return 0;
	}
}


int call_auspack(const char *arcer, const char *arcfile)
{
	char arg[20];
	pid_t c_pid;

	strcpy(arg, arcer);
	strupr(arg);
	if ((c_pid = fork()) == 0) {
		/* Ich bin child */
		execlp(auspack, auspack, arg, arcfile, NULL);
		perror(auspack);

		return 1;
	} else {
#if defined (NEXTSTEP)
		union wait stat;
#else
		int stat;
#endif
		wait(&stat);

		if (WIFEXITED(stat))
			return WEXITSTATUS(stat);
		else
			return 0;
	}
}


int call_import(const char *sysname, int net38format)
{
	pid_t c_pid;

	if ((c_pid = fork()) == 0) {
		/* Ich bin child */
		execlp(import_prog, import_prog, sysname, "ZCONNECT", NULL);
		perror(import_prog);
		return 1;
	} else {
#if defined (NEXTSTEP)
		union wait stat;
#else
		int stat;
#endif
		wait(&stat);

		if (WIFEXITED(stat))
			return WEXITSTATUS(stat);
		else
			return 0;
	}
	return 0;
}


