/*
 *  run_as - run a program as a different user
 *  Copyright (C) 2002 Matthias Andree <matthias.andree@gmx.de>
 *
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*@unused@*/ static const char id[] = "$Id$";

/*@noreturn@*/ static void usage(const char *n) {
	printf("Usage: %s useraccount /path/program [args]\n", n);
	exit(1);
}

/*@noreturn@*/ static void die(void) {
	exit(2);
}

/*@noreturn@*/ static void bail(const char *n) {
	perror(n);
	die();
}

int main(int argc, char **argv)
{
	int a = 1;
	struct passwd *p;

	if (argc < 3 || (argc > a && (!strcmp("-h", argv[a])
	|| !strcmp("--help", argv[a])))) usage(argv[0]);

	if (!strcmp("--", argv[a])) a++;

	if (!(p=getpwnam(argv[a]))) { 
		fprintf(stderr, "%s: No such user.\n", argv[0]); 
		die(); 
	}
	if (initgroups(p->pw_name, p->pw_gid)) bail(argv[0]);
	endgrent();

	if (setuid(p->pw_uid)) bail(argv[0]);
	endpwent();

	++a;
	if (execv(argv[a], argv + a)) bail(argv[a]);
	exit(0);
}
