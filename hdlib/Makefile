# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-1994  Martin Husemann
#   Copyright (C) 1995-1998  Christopher Creutzig
#   Copyright (C) 1999-2000  Dirk Meyer
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

# Makefile fuer die Header-Verwaltung

LIBS	= ../lib/strlib.a

GFLAGS	= -L ANSI-C -C -D -G -K header -N predefd_header -t

OBJS	= hash.o delheader.o find.o freepara.o joinheader.o rdpara.o \
	  wrpara.o pkread.o pkwrite.o crc.o uurd.o smtprd.o

all:    uuheader.a

install:	all

uninstall:

clean:
	-rm -f $(CLEANFILES) header.tmp header.gtmp
	-rm -f uurd.c smtprd.c pkread.c pkwrite.c
	-rm -f testheader test.out

distclean:	clean

maintainer-clean:	distclean
	-rm -f hd_def.h hd_nam.h hd_nlist.h
	-rm -f hash.c header.gperf

newhash:	maintainer-clean hash.c

uurd.c:	uurdpara.c
	@-rm -f uurd.c
	${LN} uurdpara.c uurd.c

smtprd.c:	uurdpara.c
	@-rm -f smtprd.c
	( echo "#define SMTP"; cat uurdpara.c ) >$@

pkread.c:	rdpara.c
	@-rm -f pkread.c
	( echo "#define DO_CRC"; cat rdpara.c ) >$@

pkwrite.c:	wrpara.c
	@-rm -f pkwrite.c
	( echo "#define DO_CRC"; cat wrpara.c ) >$@

hash.c:	header.gperf hash.epilogue gperf.prefix
	cat gperf.prefix header.gperf > header.gtmp
	-gperf $(GFLAGS) < header.gtmp > hash.tmp
	if test -s hash.tmp; then \
		cat hash.tmp hash.epilogue > hash.c; \
	fi
	@-rm -f header.gtmp hash.tmp

header.gperf hd_def.h hd_nam.h hd_nlist.h:	header.lst
	sed -e '/^#/d' -e '/^$$/d' < header.lst > header.tmp
	awk '{printf("HN_%s,\n", $$2)}' header.tmp > hd_nlist.h
	awk '{printf("#define HN_%s \"%s\"\n", $$2, $$1)}' header.tmp > hd_nam.h
	awk '{printf("#define HD_%s %d\n", $$2, nr); nr++}' header.tmp > hd_def.h
	awk '{printf("%s, HD_%s\n", $$1, $$2)}' header.tmp | tr '[a-z]' '[A-Z]' > header.gperf
	@-rm -f header.tmp

testheader:	testheader.o uuheader.a $(LIBS) Makefile
	$(CC) $(CFLAGS) -o testheader testheader.o uuheader.a $(LIBS) $(LDFLAGS)

check:	testheader test.in uuheader.a
	./testheader test.in test.out
	diff -p test.out test.erg

uuheader.a:	$(OBJS)
	@-rm -f uuheader.a
	ar r uuheader.a $(OBJS)
	$(RANLIB) uuheader.a

# automatic depend
crc.o: crc.c ../include/crc.h
delheader.o: delheader.c ../include/crc.h ../include/header.h \
 ../include/utility.h
find.o: find.c ../include/crc.h ../include/header.h
freepara.o: freepara.c ../include/crc.h ../include/header.h \
 ../include/utility.h
hash.o: hash.c ../include/istring.h hd_def.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h hd_nam.h \
 hd_nlist.h
joinheader.o: joinheader.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h
pkread.o: pkread.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h
pkwrite.o: pkwrite.c ../include/crc.h ../include/header.h hd_nam.h
rdpara.o: rdpara.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h
smtprd.o: smtprd.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h \
 ../include/sysexits2.h
testheader.o: testheader.c ../include/istring.h ../include/utility.h \
 ../include/uulog.h ../include/crc.h ../include/header.h hd_def.h \
 hd_nam.h
uurd.o: uurd.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h \
 ../include/sysexits2.h
uurdpara.o: uurdpara.c ../include/istring.h ../include/utility.h \
 ../include/crc.h ../include/header.h ../include/uulog.h \
 ../include/sysexits2.h
wrpara.o: wrpara.c ../include/crc.h ../include/header.h hd_nam.h
# eof
