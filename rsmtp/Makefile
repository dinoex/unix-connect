#
#	Id$: GNUmakefile	1.71	26.07.1999
#
#	(c) 1994-99, Dirk Meyer, Im Grund 4, 34317 Habichtswald
#
# makefile for dinoex

# FREEBSD	= YES
LINUX		= YES

SPOOLDIR= /var/spool/uconnect/rsmtp
DESTDIR	= /usr/local/libexec/unix-connect
BINOWN	= -o root -g wheel
BINMODE	= -m 755

include ../.makefile.conf

RSMTPSPOOL=$(SPOOLDIR)/rsmtp

WARNALL	= \
	-W \
	-Wimplicit \
	-Wreturn-type \
	-Wunused \
	-Wswitch \
	-Wcomment \
	-Wtrigraphs \
	-Wformat \
	-Wchar-subscripts \
	-Wuninitialized \
	-Wparentheses

WARNGCC24x = \
	-pedantic \
	-Wconversion \
	-Wtraditional \
	-Wenum-clash \

WARNGCC25x = \
	-Wconversion \
	-Wenum-clash \

WARNGCC26x = \
	-Wmissing-declarations \
	-pedantic \
	-Wtraditional \
	-Wenum-clash \

WARNGCC27x = \
	-Wmissing-declarations \
	-pedantic

WARNEGCS = \
	-Wimplicit-int \
	-Wimplicit-function-delcaration \
	-Wmissing-declarations \

WARNBASE = \
	$(WARNALL) \
	-Wshadow \
	-Wid-clash-30 \
	-Wpointer-arith \
	-Wcast-qual \
	-Wcast-align \
	-Wwrite-strings \
	-Waggregate-return \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Woverloaded-virtual \
	-Winline

WARNEXT	= \
	$(WARN) \
	-Wredundant-decls \
	-Wbad-function-cast \
	-Wno-import \
	-Wno-parentheses \
	-Wtest

CPPFLAGS+= -DUNIX -DHAVE_SYSEXITS
CFLAGS	= -O2 -g $(WARN)
LDFLAGS	= -g
INSTALL	= install -c

CPPFLAGS+= -DMAILDIR="\"$(RSMTPSPOOL)"\"

TARMIN	= \
	rsmtp \
	bsmtp


BINMIN	= \
	$(DESTDIR)/rsmtp \
	$(DESTDIR)/bsmtp

# rules

all:	$(TARMIN)
	date

clean:
	rm -rf *.o *.ln $(TARMIN)

clean.bin:
	rm -rf $(TARMIN)

install:	${DESTDIR} \
		$(TARMIN) \
		$(BINMIN)

rsmtp:		rsmtp.o
bsmtp:		bsmtp.o

rsmtp.o:	rsmtp.c version.h
bsmtp.o:	bsmtp.c version.h

# install rules

$(DESTDIR):
	mkdir -p ${DESTDIR}

$(DESTDIR)/rsmtp: rsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) $(<) $(@)
	$(INSTALL) $(BINMODE) $(BINOWN) $(<) $(DESTDIR)/rgsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) $(<) $(DESTDIR)/rcsmtp

$(DESTDIR)/bsmtp: bsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) $(<) $(@)

# end of makefile
