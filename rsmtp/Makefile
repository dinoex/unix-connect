#
#	Id$: GNUmakefile	1.71	08.08.1999
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

include ../Makefile.global

RSMTPSPOOL=$(SPOOLDIR)/rsmtp

WARNALL?	= \
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

WARNGCC24x? = \
	-pedantic \
	-Wconversion \
	-Wtraditional \
	-Wenum-clash \

WARNGCC25x? = \
	-Wconversion \
	-Wenum-clash \

WARNGCC26x? = \
	-Wmissing-declarations \
	-pedantic \
	-Wtraditional \
	-Wenum-clash \

WARNGCC27x? = \
	-Wmissing-declarations \
	-pedantic

WARNEGCS? = \
	-Wimplicit-int \
	-Wimplicit-function-delcaration \
	-Wmissing-declarations \

WARNBASE? = \
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

WARN?	= \
	$(WARNBASE) \
	$(WARNEGCS)

CPPFLAGS+= -DUNIX -DHAVE_SYSEXITS
CFLAGS+= -O2 -g $(WARN)
LDFLAGS+= -g
INSTALL= install -c

CPPFLAGS+= -DMAILDIR="\"$(RSMTPSPOOL)"\"

TARMIN	= \
	rsmtp \
	bsmtp


BINMIN	= \
	$(DESTDIR)/rsmtp \
	$(DESTDIR)/bsmtp

# rules

all:	$(TARMIN)

install:	${DESTDIR} \
		$(SPOOLDIR)/rsmtp \
		$(TARMIN) \
		$(BINMIN)

clean:
	-rm -f $(CLEANFILES) $(TARMIN)

distclean:	clean

rsmtp:	rsmtp.o
	$(CC) -o rsmtp rsmtp.o $(LIBS) $(LDFLAGS)

bsmtp:	bsmtp.o
	$(CC) -o bsmtp bsmtp.o $(LIBS) $(LDFLAGS)

# install rules

$(DESTDIR):
	mkdir -p ${DESTDIR}

$(SPOOLDIR)/rsmtp:
	mkdir -p $(SPOOLDIR)/rsmtp
	chown $(USER) $(SPOOLDIR)/rsmtp
	chmod 1777 $(SPOOLDIR)/rsmtp

$(DESTDIR)/rsmtp: rsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) rsmtp $@
	$(INSTALL) $(BINMODE) $(BINOWN) rsmtp $(DESTDIR)/rgsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) rsmtp $(DESTDIR)/rcsmtp

$(DESTDIR)/bsmtp: bsmtp
	$(INSTALL) $(BINMODE) $(BINOWN) bsmtp $@

.c.o:	../Makefile.global
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

# end of makefile
# automatic depend
bsmtp.o: bsmtp.c version.h
rsmtp.o: rsmtp.c version.h
# eof
