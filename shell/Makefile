# Makefile zu "ZCONNECT fuer UNIX"
# zustaendig fuer das Directory shell

SHELL    = /bin/sh

CC       = gcc
CFLAGS   = -g -Wall
LDFLAGS  = 
RANLIB   = ranlib
DESTDIR  = /usr/local/lib/zconnect
SPOOLDIR = /var/spool/zconnect
FQDN     = nescio.foebud.org
USER     = mbox
PATHADD	 = /usr/local/bin:/usr/lib/news:/usr/local/lib/news/bin
APCOBJS  = 


# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-94  Martin Husemann
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#  ------------------------------------------------------------------------
#  Eine deutsche Zusammenfassung zum Copyright dieser Programme sowie der
#  GNU General Public License finden Sie in der Datei README.1st.
#  ------------------------------------------------------------------------
#
#  Bugreports, suggestions for improvement, patches, ports to other systems
#  etc. are welcome. Contact the maintainer by e-mail:
#  christopher@nescio.foebud.org or snail-mail:
#  Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
#
#  There is a mailing-list for user-support:
#   unix-connect@mailinglisten.im-netz.de,
#  write a mail with subject "Help" to
#   unix-connect-request@mailinglisten.im-netz.de
#  for instructions on how to join this list.
#


install:
	for file in *.SH; do \
		OUT=$(DESTDIR)/`basename $$file .SH`; \
		sed -e 's:@DESTDIR@:'$(DESTDIR)':g' \
		-e 's:@SPOOLDIR@:'$(SPOOLDIR)':g' \
		-e 's/@FQDN@/'$(FQDN)'/g' \
		-e 's#@PATHADD@#'$(PATHADD)'#g' \
		< $$file > $$OUT; \
		chmod a+x $$OUT; \
		chown $(USER) $$OUT; \
	done
