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
#   nora.e@mailinglisten.im-netz.de
#  for instructions on how to join this list.
#

include ../Makefile.global

OBJS	= hayes.o pattern.o stricmp.o track.o uufatal.o utility.o strlwr.o \
	  strupr.o strdup.o rawtty.o str2eda.o ministat.o lockwait.o uulog.o \
	  trap.o

STROBJS	= stricmp.o uufatal.o utility.o strlwr.o strupr.o strdup.o

all:	gatelib.a strlib.a

install:	all

clean:
	-rm -f $(CLEANFILES)

clobber:	clean

tidy:		clobber

gatelib.a:	$(OBJS)
	ar r gatelib.a $(OBJS)
	$(RANLIB) gatelib.a

strlib.a:	$(STROBJS)
	ar r strlib.a $(STROBJS)
	$(RANLIB) strlib.a

.c.o:	../Makefile.global
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

# Individuelle Abhaengigkeiten [gcc -I../include -MM *.c]

hayes.o: hayes.c ../include/config.h ../include/sysdep.h \
 ../include/header.h ../include/crc.h ../include/hd_def.h \
 ../include/hd_nam.h ../include/ministat.h ../include/boxstat.h \
 ../include/hayes.h ../include/track.h
lockwait.o: lockwait.c ../include/config.h ../include/sysdep.h
ministat.o: ministat.c ../include/config.h ../include/sysdep.h \
 ../include/ministat.h ../include/boxstat.h ../include/header.h \
 ../include/crc.h ../include/hd_nam.h ../include/hd_def.h \
 ../include/uulog.h ../include/lib.h
pattern.o: pattern.c ../include/config.h ../include/sysdep.h \
 ../include/pattern.h ../include/lib.h
rawtty.o: rawtty.c ../include/config.h ../include/sysdep.h \
 ../include/line.h
str2eda.o: str2eda.c ../include/config.h ../include/sysdep.h
strdup.o: strdup.c ../include/config.h ../include/sysdep.h \
 ../include/uulog.h
stricmp.o: stricmp.c ../include/config.h ../include/sysdep.h
strlwr.o: strlwr.c ../include/config.h ../include/sysdep.h
strupr.o: strupr.c ../include/config.h ../include/sysdep.h
track.o: track.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/track.h
trap.o: trap.c ../include/config.h ../include/sysdep.h \
 ../include/trap.h ../include/ministat.h ../include/boxstat.h \
 ../include/header.h ../include/crc.h
utility.o: utility.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/boxstat.h ../include/uulog.h
uufatal.o: uufatal.c ../include/config.h ../include/sysdep.h \
 ../include/ministat.h ../include/boxstat.h ../include/header.h \
 ../include/crc.h ../include/uulog.h
uulog.o: uulog.c ../include/config.h ../include/sysdep.h \
 ../include/uulog.h ../include/ministat.h ../include/boxstat.h \
 ../include/header.h ../include/crc.h