# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-94  Martin Husemann
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

GATEOBJS= hayes.o pattern.o track.o rawtty.o lockwait.o \
	  trap.o readonedir.o backup.o uudebug.o ministat.o

MINIOBJS= ministat.o

STROBJS	= utility.o stricmp.o strlwr.o strupr.o strdup.o stccpy.o \
	  uufatal.o uulog.o

all:	strlib.a ministat.a $(ONLINE_LIB)

install:	all

uninstall:

clean:
	-rm -f $(CLEANFILES) gatelib.a strlib.a

distclean:	clean

gatelib.a:	$(GATEOBJS) Makefile
	-rm -f gatelib.a
	ar r gatelib.a $(GATEOBJS)
	$(RANLIB) gatelib.a

ministat.a:	$(MINIOBJS) Makefile
	-rm -f ministat.a
	ar r ministat.a $(MINIOBJS)
	$(RANLIB) ministat.a

strlib.a:	$(STROBJS) Makefile
	-rm -f strlib.a
	ar r strlib.a $(STROBJS)
	$(RANLIB) strlib.a

# automatic depend
backup.o: backup.c ../include/utility.h ../include/calllib.h \
 ../include/uulog.h ../include/uudebug.h
hayes.o: hayes.c ../include/config.h ../include/sysdep.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/ministat.h ../include/hayes.h \
 ../include/track.h
lockwait.o: lockwait.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h
ministat.o: ministat.c ../include/utility.h ../include/boxstat.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_nam.h \
 ../hdlib/hd_def.h ../include/uulog.h ../include/ministat.h
pattern.o: pattern.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/pattern.h
rawtty.o: rawtty.c ../include/config.h ../include/sysdep.h \
 ../include/line.h
readonedir.o: readonedir.c ../include/utility.h ../include/calllib.h \
 ../include/uulog.h
stccpy.o: stccpy.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h
strdup.o: strdup.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/uulog.h
stricmp.o: stricmp.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h
strlwr.o: strlwr.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h
strupr.o: strupr.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h
track.o: track.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/track.h
trap.o: trap.c ../include/config.h ../include/sysdep.h \
 ../include/trap.h ../include/ministat.h
utility.o: utility.c ../include/config.h ../include/sysdep.h \
 ../include/boxstat.h ../include/uulog.h ../include/utility.h
uudebug.o: uudebug.c ../include/config.h ../include/sysdep.h \
 ../include/uulog.h ../include/uudebug.h
uufatal.o: uufatal.c ../include/uulog.h ../include/sysexits2.h
uulog.o: uulog.c ../include/uulog.h
# eof
