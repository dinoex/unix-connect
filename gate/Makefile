# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1995-98  Christopher Creutzig
#   Copyright (C) 1999     Dirk Meyer
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
#   etc. are welcome. Contact the maintainer by e-mail:
#   dirk.meyer@dinoex.sub.org or snail-mail:
#   Dirk Meyer, Im Grund 4, 34317 Habichtswald
#
#   There is a mailing-list for user-support:
#    unix-connect@mailinglisten.im-netz.de,
#   write a mail with subject "Help" to
#    nora.e@mailinglisten.im-netz.de
#   for instructions on how to join this list.
#

include ../Makefile.global

# Makefile fuer den UNIX-Connect Gateway

COMMON	= mime.o alias.o gtools.o
LIBS	= ../lib/ministat.a ../hdlib/uuheader.a ../lib/strlib.a
DATELIB	= ../date/datelib.a
PROGS	= uurnews uursmtp uuwsmtp uuwnews
TPROGS	= testeda testmime

all:	$(PROGS)

install:	all
	$(INSTALL_PROGRAM) uursmtp uuwsmtp uuwnews $(BINDIR)
	$(INSTALL_PROGRAM) -m 4555 uurnews $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(PROGS)

clean:
	-rm -f $(CLEANFILES) $(PROGS) $(TPROGS)

distclean:	clean

check:	testeda
	./testeda

uursmtp:	uursmtp.o uuconv.o iso2pc.o $(COMMON) $(APCOBJS) $(DATELIB) $(LIBS) Makefile
	$(CC) $(MODELL) -o $@ $@.o uuconv.o iso2pc.o $(COMMON) $(APCOBJS) $(DATELIB) $(LIBS) $(LDFLAGS)

uuwsmtp:	uuwsmtp.o zconv.o pc2iso.o $(COMMON) $(LIBS) Makefile
	$(CC) $(MODELL) -o $@ $@.o zconv.o pc2iso.o $(COMMON) $(LIBS) $(LDFLAGS)

uuwnews:	uuwnews.o zconv.o uuapprove.o pc2iso.o $(COMMON) $(LIBS) Makefile
	$(CC) $(MODELL) -o $@ $@.o zconv.o uuapprove.o pc2iso.o $(COMMON) $(LIBS) $(LDFLAGS)

uurnews:	uurnews.o uuconv.o iso2pc.o $(COMMON) $(APCOBJS) $(DATELIB) $(LIBS) Makefile
	$(CC) $(MODELL) -o $@ $@.o uuconv.o iso2pc.o $(COMMON) $(APCOBJS) $(DATELIB) $(LIBS) $(LDFLAGS)

testmime:	testmime.o mime.o $(DATELIB) $(LIBS) Makefile
	$(CC) $(MODELL) -o $@ $@.o mime.o $(DATELIB) $(LIBS) \
	-lreadline -ltermcap $(LDFLAGS)

testeda:	testeda.o $(DATELIB) Makefile
	$(CC) $(CFLAGS) -o testeda testeda.o $(DATELIB) $(LDFLAGS)

# automatic depend
alias.o: alias.c ../include/utility.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/uulog.h ../include/alias.h ../include/sysdep.h \
 ../include/ministat.h
apc_a2b.o: apc_a2b.c ../include/apc_a2b.h
atob.o: atob.c
gtools.o: gtools.c ../include/gtools.h ../include/version.h \
 ../include/sysexits2.h
iso2pc.o: iso2pc.c ../include/lib.h
mime.o: mime.c ../include/utility.h ../include/crc.h \
 ../include/header.h ../include/mime.h ../hdlib/hd_def.h \
 ../include/uulog.h ../include/sysexits2.h
pc2iso.o: pc2iso.c ../include/lib.h
testeda.o: testeda.c ../include/datelib.h
testmime.o: testmime.c
uuapprove.o: uuapprove.c ../include/utility.h ../include/uulog.h \
 ../include/uuapprove.h
uuconv.o: uuconv.c ../include/utility.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/boxstat.h ../include/version.h ../include/uulog.h \
 ../include/alias.h ../include/sysdep.h ../include/mime.h \
 ../include/uuconv.h ../include/lib.h ../include/datelib.h
uurnews.o: uurnews.c ../include/utility.h ../include/crc.h \
 ../include/header.h ../include/uulog.h ../hdlib/hd_nam.h \
 ../hdlib/hd_def.h ../include/boxstat.h ../include/ministat.h \
 ../include/mime.h ../include/lib.h ../include/uuconv.h \
 ../include/gtools.h ../include/sysexits2.h
uursmtp.o: uursmtp.c ../include/boxstat.h ../include/ministat.h \
 ../include/utility.h ../include/crc.h ../include/header.h \
 ../hdlib/hd_nam.h ../hdlib/hd_def.h ../include/uulog.h \
 ../include/mime.h ../include/lib.h ../include/uuconv.h \
 ../include/gtools.h ../include/sysexits2.h
uuwnews.o: uuwnews.c ../include/utility.h ../include/lib.h \
 ../include/boxstat.h ../include/ministat.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/uuapprove.h ../include/uulog.h ../include/mime.h \
 ../include/zconv.h ../include/gtools.h ../include/sysexits2.h
uuwsmtp.o: uuwsmtp.c ../include/utility.h ../include/lib.h \
 ../include/boxstat.h ../include/ministat.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/uulog.h ../include/mime.h ../include/zconv.h \
 ../include/gtools.h ../include/sysexits2.h
zconv.o: zconv.c ../include/utility.h ../include/lib.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/uulog.h ../include/alias.h \
 ../include/sysdep.h ../include/version.h ../include/boxstat.h \
 ../include/mime.h ../include/zconv.h
# eof
