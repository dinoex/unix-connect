# $Id$
#
#   UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
#   Copyright (C) 1993-1994  Martin Husemann
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

FILESKEEP = anruf
FILESFORCE = auspack einpack import prearc uumlist \
	cron.d.hourly logrotate.d.entry
FILES = $(FILESKEEP) $(FILESFORCE)

all:	$(FILES)

anruf: anruf.in  ../Makefile.global
	$(SHELL) ../helpers/subst < anruf.in > $@

auspack: auspack.in  ../Makefile.global
	$(SHELL) ../helpers/subst < auspack.in > $@

einpack: einpack.in  ../Makefile.global
	$(SHELL) ../helpers/subst < einpack.in > $@

import: import.in  ../Makefile.global
	$(SHELL) ../helpers/subst < import.in > $@

prearc: prearc.in  ../Makefile.global
	$(SHELL) ../helpers/subst < prearc.in > $@

uumlist: uumlist.in  ../Makefile.global
	$(SHELL) ../helpers/subst < uumlist.in > $@

cron.d.hourly: cron.d.hourly.in  ../Makefile.global
	$(SHELL) ../helpers/subst < cron.d.hourly.in > $@

logrotate.d.entry: logrotate.d.entry.in  ../Makefile.global
	$(SHELL) ../helpers/subst < logrotate.d.entry.in > $@

install: $(FILES)
	for i in $(FILESKEEP) ; do \
		if [ ! -e $(BINDIR)/$$i ] ; then \
			$(INSTALL_PROGRAM) $$i $(BINDIR) ; \
		fi ; \
	done
	$(INSTALL_PROGRAM) $(FILESFORCE) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(FILES)

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f $(FILES)

# BSD
#
#$(FILES): $(.PREFIX).in
#	$(SHELL) ../helpers/subst < $(.PREFIX).in > $(.TARGET)
#
# GNU
#
#$(FILES): % : %.in
#	$(SHELL) ../helpers/subst < $< > $@
#
