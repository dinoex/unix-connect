# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-1994  Martin Husemann
#   Copyright (C) 1995       Christopher Creutzig
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
#  Bugreports, suggestions for improvement, patches, ports to other systems
#  etc. are welcome. Contact the maintainer by e-mail:
#  christopher@nescio.foebud.org or snail-mail:
#  Christopher Creutzig, Im Samtfelde 19, 33098 Paderborn
#
#  There is a mailing-list for user-support:
#   unix-connect-users@lists.sourceforge.net,
#  write a mail with subject "Help" to
#   unix-connect-users-request@lists.sourceforge.net
#  for instructions on how to join this list.
#

include ../Makefile.global

LIBS	= ../lib/gatelib.a ../lib/ministat.a \
	  ../hdlib/uuheader.a ../lib/strlib.a

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

receiver:	$(ROBJS) $(LIBS) Makefile
	$(CC) -o receiver $(ROBJS) $(LIBS) $(LDFLAGS)

zcall:	$(COBJS) $(LIBS) Makefile
	$(CC) -o zcall $(COBJS) $(LIBS) $(LDFLAGS)

januslogin:	januslogin.o auth.o xprog.o locknew.o lockdev.o lock.o import.o $(LIBS) Makefile
	$(CC) -o januslogin januslogin.o auth.o xprog.o locknew.o lockdev.o lock.o import.o $(LIBS) $(LDFLAGS)

testrun: testrun.o $(LIBS) Makefile
	$(CC) -o testrun testrun.o $(LIBS) $(LDFLAGS)

check:	testrun
	./testrun 2>&1 | grep -v ^cwd >test.out
	diff -p test.out test.erg

# automatic depend
action.o: action.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h ../include/xprog.h
auth.o: auth.c ../include/auth.h ../include/zconnect.h \
 ../include/istring.h ../include/crc.h ../include/header.h \
 ../hdlib/hd_def.h ../hdlib/hd_nam.h ../include/version.h \
 ../include/track.h ../include/line.h ../include/boxstat.h \
 ../include/ministat.h ../include/uulog.h ../include/locknames.h \
 ../include/utility.h ../include/idir.h
call.o: call.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h ../include/hayes.h ../include/xprog.h \
 ../include/locknames.h ../include/calllib.h
common.o: common.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
filedialog.o: filedialog.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
filemaster.o: filemaster.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
import.o: import.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/idir.h ../include/utility.h ../include/xprog.h \
 ../include/calllib.h ../include/uudebug.h
januslogin.o: januslogin.c ../include/auth.h ../include/config.h \
 ../include/sysdep.h ../include/zconnect.h ../include/istring.h \
 ../include/crc.h ../include/header.h ../hdlib/hd_def.h \
 ../hdlib/hd_nam.h ../include/version.h ../include/track.h \
 ../include/line.h ../include/boxstat.h ../include/ministat.h \
 ../include/uulog.h ../include/utility.h ../include/xprog.h \
 ../include/locknames.h ../include/locknew.h ../include/calllib.h
lock.o: lock.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h ../include/policy.h
lockdev.o: lockdev.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h ../include/policy.h
locknew.o: locknew.c ../include/locknew.h ../include/locknames.h \
 ../include/utility.h
myself.o: myself.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h
pingpong.o: pingpong.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
prepare.o: prepare.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h ../include/xprog.h ../include/locknames.h
prologue.o: prologue.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h
receiver.o: receiver.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h
sysdialog.o: sysdialog.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
sysmaster.o: sysmaster.c ../include/config.h ../include/sysdep.h \
 ../include/zconnect.h ../include/istring.h ../include/crc.h \
 ../include/header.h ../hdlib/hd_def.h ../hdlib/hd_nam.h \
 ../include/version.h ../include/track.h ../include/line.h \
 ../include/boxstat.h ../include/ministat.h ../include/uulog.h \
 ../include/utility.h
testrun.o: testrun.c xprog.c ../include/config.h ../include/sysdep.h \
 ../include/iwait.h ../include/istring.h ../include/utility.h \
 ../include/boxstat.h ../include/ministat.h ../include/xprog.h \
 ../include/uudebug.h ../include/uulog.h
xprog.o: xprog.c ../include/config.h ../include/sysdep.h \
 ../include/iwait.h ../include/istring.h ../include/utility.h \
 ../include/boxstat.h ../include/ministat.h ../include/xprog.h \
 ../include/uudebug.h ../include/uulog.h
# eof
