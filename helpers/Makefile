#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.

#   Dieses Makefile (c) Andreas Barth <aba@muenchen.pro-bahn.de> 1999
#   Die einzelnen Programme sind einzeln (c).

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

SHELL=/bin/sh
CLEANFILES = *.o *.a *.bak *.rej *.orig core *.core
CDIRS = date hdlib lib gate online tools rsmtp
SUBDIRS = $(CDIRS) etc shell mail-and-news

all: subst.in ../configure

install:

clean:

distclean:
	-rm -f $(CLEANFILES)
	-rm -f subst
	-cd .. && rm -f $(CLEANFILES)
	-cd .. && cp -f Makefile.global.in Makefile.global
	for dir in $(SUBDIRS); do \
		(cd ../$$dir && $(MAKE) -$(MAKEFLAGS) distclean) done
	-cd .. && rm -f Makefile.global Makefile subst
	-cd .. && rm -f config.cache config.log config.status confdefs.h
	-cd .. && rm -f include/sysdep.h include/policy.h

maintainer-clean:
	-rm -f subst subst.in ../configure
	-cd .. && cp -f Makefile.global.in Makefile.global
	cd ../hdlib && $(MAKE) -$(MAKEFLAGS) maintainer-clean

dist:	all
	-cd .. && cp -f Makefile.global.in Makefile.global
	cd ../hdlib && $(MAKE) -$(MAKEFLAGS) hash.c
	-cd .. && rm -f Makefile.global
	dir=unix-connect-`awk '/VERSION/{gsub("\"",""); print $$3; exit}' \
		../include/version.h` && \
	/data/batch/xcopy-cvs .. /tmp/$${dir} && \
	cd /tmp && \
	rm -f $${dir}.tar $${dir}.tar.gz && \
	tar -cf $${dir}.tar $${dir} && \
	gzip -9 $${dir}.tar && \
	rm -rf $${dir}

subst.in: makesubst ../configure.in
	$(SHELL) ./makesubst > subst.in
	chmod a+x subst.in

../configure:	../configure.in subst.in
	cd .. && autoconf

# do or redo configuration
../Makefile: ../Makefile.in ../config.status
	cd .. && CONFIG_FILES=Makefile CONFIG_HEADERS= $(SHELL) ./config.status

../Makefile.global: ../Makefile.global.in ../config.status
	cd .. && CONFIG_FILES=Makefile.global CONFIG_HEADERS= $(SHELL) ./config.status

subst: subst.in ../config.status
	cd .. && CONFIG_FILES=helpers/subst CONFIG_HEADERS= $(SHELL) ./config.status

# default only with the minimum parametes
../config.status:	../configure
	cd .. && sh configure --with-user=news --with-group=uucp

# eof
