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

all: subst.in ../configure

install:

clean:

distclean:	clean
	-rm -f subst subst.in ../configure

subst.in: makesubst ../configure.in
	$(SHELL) ./makesubst > subst.in
	chmod a+x subst.in

../configure:	../configure.in
	(cd ..;  autoconf)

# eof
