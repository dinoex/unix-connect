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
#   Dirk Meyer, Im Grund 4, 34317 Habichstwald
#
#   There is a mailing-list for user-support:
#    unix-connect@mailinglisten.im-netz.de,
#   write a mail with subject "Help" to
#    nora.e@mailinglisten.im-netz.de
#   for instructions on how to join this list.
#

include ../Makefile.global

# Makefile fuer den UNIX-Connect Gateway

LIBS	=	../lib/ministat.o ../hdlib/uuheader.a ../lib/gatelib.a
DATELIB	=	../date/getindate.o ../date/datetok.o

all:	uurnews uursmtp uuwsmtp uuwnews

install:	all
	cp uurnews uursmtp uuwsmtp uuwnews $(DESTDIR)
	chown $(USER) $(DESTDIR)/uursmtp
	chown $(USER) $(DESTDIR)/uurnews
	chown $(USER) $(DESTDIR)/uuwnews
	chown $(USER) $(DESTDIR)/uuwsmtp
	chmod +s $(DESTDIR)/uursmtp $(DESTDIR)/uurnews
	chmod +s $(DESTDIR)/uuwsmtp $(DESTDIR)/uuwnews

clean:
	-rm -f $(CLEANFILES)

clobber:	clean
	-rm -f uurnews uursmtp uuwsmtp uuwnews

tidy:		clobber

uursmtp:	uursmtp.o uuconv.o mime.o alias.o iso2pc.o $(APCOBJS) $(DATELIB) $(LIBS)
	$(CC) $(MODELL) -o $@ $@.o uuconv.o mime.o alias.o iso2pc.o $(APCOBJS) $(DATELIB) $(LIBS) $(LDFLAGS)

uuwsmtp:	uuwsmtp.o zconv.o mime.o alias.o pc2iso.o $(LIBS)
	$(CC) $(MODELL) -o $@ $@.o zconv.o mime.o alias.o pc2iso.o $(LIBS) $(LDFLAGS)

uuwnews:	uuwnews.o zconv.o mime.o alias.o uuapprove.o pc2iso.o $(LIBS)
	$(CC) $(MODELL) -o $@ $@.o zconv.o mime.o alias.o uuapprove.o pc2iso.o $(LIBS) $(LDFLAGS)

uurnews:	uurnews.o uuconv.o mime.o alias.o iso2pc.o $(APCOBJS) $(DATELIB) $(LIBS)
	$(CC) $(MODELL) -o $@ $@.o uuconv.o mime.o alias.o iso2pc.o $(APCOBJS) $(DATELIB) $(LIBS) $(LDFLAGS)

testmime:	testmime.o mime.o $(DATELIB) $(LIBS)
	$(CC) $(MODELL) -o $@ $@.o mime.o $(DATELIB) $(LIBS) \
	 -lreadline -ltermcap $(LDFLAGS)

.c.o:	../Makefile.global
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

# automatic depend
alias.o: alias.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/lib.h ../include/alias.h \
 ../include/header.h ../include/crc.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/uulog.h
apc_a2b.o: apc_a2b.c ../include/apc_a2b.h
atob.o: atob.c
iso2pc.o: iso2pc.c
mime.o: mime.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/header.h ../include/crc.h \
 ../include/mime.h ../hdlib/hd_def.h ../include/uulog.h
pc2iso.o: pc2iso.c
testmime.o: testmime.c
uuapprove.o: uuapprove.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/uulog.h ../include/uuapprove.h
uuconv.o: uuconv.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/header.h ../include/crc.h \
 ../hdlib/hd_def.h ../hdlib/hd_nam.h ../include/boxstat.h \
 ../include/version.h ../include/uulog.h ../include/alias.h \
 ../include/mime.h ../include/uuconv.h ../include/apc_a2b.h \
 ../include/lib.h ../include/datelib.h
uurnews.o: uurnews.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/utility.h ../include/header.h \
 ../include/crc.h ../include/uulog.h ../hdlib/hd_nam.h \
 ../hdlib/hd_def.h ../include/version.h ../include/ministat.h \
 ../include/boxstat.h ../include/mime.h ../include/uuconv.h \
 ../include/apc_a2b.h
uursmtp.o: uursmtp.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/ministat.h ../include/boxstat.h \
 ../include/header.h ../include/crc.h ../include/utility.h \
 ../hdlib/hd_nam.h ../hdlib/hd_def.h ../include/version.h \
 ../include/uulog.h ../include/mime.h ../include/uuconv.h \
 ../include/apc_a2b.h
uuwnews.o: uuwnews.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/boxstat.h ../include/ministat.h \
 ../include/header.h ../include/crc.h ../include/utility.h \
 ../hdlib/hd_def.h ../hdlib/hd_nam.h ../include/version.h \
 ../include/uuapprove.h ../include/uulog.h ../include/mime.h \
 ../include/zconv.h
uuwsmtp.o: uuwsmtp.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/boxstat.h ../include/ministat.h \
 ../include/header.h ../include/crc.h ../include/utility.h \
 ../hdlib/hd_def.h ../hdlib/hd_nam.h ../include/version.h \
 ../include/uulog.h ../include/zconv.h ../include/mime.h
zconv.o: zconv.c ../include/config.h ../include/sysdep.h \
 ../include/lib.h ../include/utility.h ../include/header.h \
 ../include/crc.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/uulog.h ../include/alias.h ../include/version.h \
 ../include/boxstat.h ../include/mime.h ../include/zconv.h
# eof
