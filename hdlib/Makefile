# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-94  Martin Husemann
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

# Makefile fuer die Header-Verwaltung

LIBS	= ../lib/strlib.a

GFLAGS	= -L ANSI-C -C -D -G -K header -N predefd_header -t

OBJS	= hash.o delheader.o find.o freepara.o joinheader.o rdpara.o \
	  wrpara.o pkread.o pkwrite.o crc.o uurd.o smtprd.o

all:    uuheader.a $(LIBS)

install:	all

clean:
	-rm -f $(CLEANFILES) header.tmp header.gtmp test.out

clobber:	clean
	-rm -f testheader

tidy:	clobber

newhash-clean:	clobber
	-rm -f hd_def.h hd_nam.h hd_nlist.h
	-rm -f testheader hash.c header.gperf

newhash:	newhash-clean hash.c

uurd.o:	uurdpara.c
	-rm -f uurd.c
	-ln uurdpara.c uurd.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c uurd.c
	-rm -f uurd.c

smtprd.o:	uurdpara.c
	-rm -f smtprd.c
	-ln uurdpara.c smtprd.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DSMTP -c smtprd.c
	-rm -f smtprd.c

pkread.o:	rdpara.c
	-rm -f pkread.c
	-ln rdpara.c pkread.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DDO_CRC -c pkread.c
	-rm -f pkread.c

pkwrite.o:	wrpara.c
	-rm -f pkwrite.c
	-ln wrpara.c pkwrite.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DDO_CRC -c pkwrite.c
	-rm -f pkwrite.c

hash.c:	header.gperf hash.epilogue gperf.prefix
	cat gperf.prefix header.gperf > header.gtmp
	-gperf $(GFLAGS) < header.gtmp > hash.tmp
	if test -s hash.tmp; \
		then cat hash.tmp hash.epilogue > hash.c; \
		else touch hash.c; fi
	-rm -f header.gtmp hash.tmp

header.gperf hd_def.h hd_nam.h hd_nlist.h:	header.lst
	sed -e '/^#/d' -e '/^$$/d' < header.lst > header.tmp
	awk '{printf("HN_%s,\n", $$2)}' header.tmp > hd_nlist.h
	awk '{printf("#define HN_%s \"%s\"\n", $$2, $$1)}' header.tmp > hd_nam.h
	awk '{printf("#define HD_%s %d\n", $$2, nr); nr++}' header.tmp > hd_def.h
	awk '{printf("%s, HD_%s\n", $$1, $$2)}' header.tmp | tr '[a-z]' '[A-Z]' > header.gperf
	-rm -f header.tmp

testheader:	testheader.o uuheader.a $(LIBS)
	$(CC) $(CFLAGS) -o testheader testheader.o uuheader.a ../lib/ministat.o $(LIBS) $(LDFLAGS)

test:	testheader test.in uuheader.a
	./testheader test.in test.out
	diff -p test.out test.erg

uuheader.a:	$(OBJS)
	ar r uuheader.a $(OBJS)
	$(RANLIB) uuheader.a

.c.o:	../Makefile.global
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

crc.o : crc.c ../include/crc.h ../include/sysdep.h
delheader.o : delheader.c ../include/header.h ../include/sysdep.h ../include/crc.h
find.o : find.c ../include/header.h ../include/sysdep.h ../include/crc.h
freepara.o : freepara.c ../include/header.h ../include/sysdep.h ../include/crc.h
hash.o : hash.c hd_def.h ../include/header.h ../include/sysdep.h ../include/crc.h \
  ../include/uulog.h hd_nam.h hd_nlist.h
joinheader.o : joinheader.c ../include/header.h ../include/sysdep.h ../include/crc.h \
  ../include/uulog.h
rdpara.o : rdpara.c ../include/header.h ../include/sysdep.h ../include/crc.h \
  ../include/uulog.h
testheader.o : testheader.c ../include/uulog.h ../include/sysdep.h ../include/header.h \
  ../include/crc.h hd_def.h hd_nam.h
uurdpara.o : uurdpara.c ../include/header.h ../include/sysdep.h ../include/crc.h \
  ../include/uulog.h
wrpara.o : wrpara.c ../include/header.h ../include/sysdep.h ../include/crc.h \
  hd_nam.h