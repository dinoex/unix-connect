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
#   Bugreports, suggestions for improvement, patches, ports to other systems
#   etc. are welcome. Contact the author by e-mail: christopher@nescio.foebud.org
#   or snail-mail: Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
#
#  There is a mailing-list for user-support:
#   unix-connect@mailinglisten.im-netz.de,
#  write a mail with subject "Help" to
#   nora.e@mailinglisten.im-netz.de
#  for instructions on how to join this list.
#

include ../Makefile.global

LIBS	= ../hdlib/uuheader.a ../lib/strlib.a ../hdlib/uuheader.a

PROGS	= zconsplit list-systems

all:	$(PROGS)

install:	all
	$(INSTALL_PROGRAM) $(PROGS) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(PROGS)

clean:
	-rm -f $(CLEANFILES) $(PROGS)

distclean:	clean

list-systems:	list-systems.o $(LIBS) Makefile
	$(CC) -o list-systems list-systems.o $(LIBS) $(LDFLAGS)

zconsplit:	zconsplit.o $(LIBS) Makefile
	$(CC) -o zconsplit zconsplit.o $(LIBS) $(LDFLAGS)

# automatic depend
list-systems.o: list-systems.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/crc.h ../include/header.h \
 ../hdlib/hd_def.h ../hdlib/hd_nam.h ../include/boxstat.h \
 ../include/ministat.h
zconsplit.o: zconsplit.c ../include/config.h ../include/sysdep.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/boxstat.h ../include/ministat.h \
 ../include/utility.h
# eof
