# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-94  Martin Husemann
#   Copyright (C) 1995 Christopher Creutzig
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

LIBS	= ../lib/ministat.o ../hdlib/uuheader.a \
	../lib/gatelib.a ../lib/strlib.a

PROGS	= zcall receiver januslogin

OBJS	= myself.o common.o pingpong.o xprog.o lockdev.o lock.o action.o \
	  prepare.o import.o

ROBJS	= receiver.o prologue.o sysdialog.o filedialog.o $(OBJS)
COBJS	= call.o sysmaster.o filemaster.o $(OBJS)

all:	$(PROGS)

install:	all
	$(INSTALL_PROGRAM) $(PROGS) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(PROGS)

clean:
	-rm -f $(CLEANFILES) $(PROGS)

distclean:	clean

receiver:	$(ROBJS) $(LIBS)
	$(CC) -o receiver $(ROBJS) $(LIBS) $(LDFLAGS)

zcall:	$(COBJS) $(LIBS)
	$(CC) -o zcall $(COBJS) $(LIBS) $(LDFLAGS)

januslogin:	januslogin.o xprog.o lockdev.o lock.o import.o $(LIBS)
	$(CC) -o januslogin januslogin.o xprog.o lockdev.o lock.o import.o $(LIBS) $(LDFLAGS)

# automatic depend
action.o: action.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/xprog.h
call.o: call.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/hayes.h \
 ../include/xprog.h locknames.h
common.o: common.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
filedialog.o: filedialog.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
filemaster.o: filemaster.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
import.o: import.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/xprog.h
januslogin.o: januslogin.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/xprog.h \
 locknames.h
lock.o: lock.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/policy.h
lockdev.o: lockdev.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/policy.h
myself.o: myself.c ../include/config.h ../include/sysdep.h zconnect.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h
pingpong.o: pingpong.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
prepare.o: prepare.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/xprog.h \
 locknames.h
prologue.o: prologue.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h
receiver.o: receiver.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h
sysdialog.o: sysdialog.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
sysmaster.o: sysmaster.c ../include/config.h ../include/sysdep.h \
 zconnect.h ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h
xprog.o: xprog.c ../include/config.h ../include/sysdep.h \
 ../include/utility.h ../include/boxstat.h ../include/ministat.h \
 ../include/xprog.h
# eof
