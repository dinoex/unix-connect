#
#	Id$: GNUmakefile	1.71	08.08.1999
#
#	(c) 1994-99, Dirk Meyer, Im Grund 4, 34317 Habichtswald
#

include ../Makefile.global

RSMTPSPOOL=$(SPOOLDIR)/rsmtp

CPPFLAGS+= -DUNIX -DHAVE_SYSEXITS
CFLAGS+= -O2 -g $(WARN)
LDFLAGS+= -g

CPPFLAGS+= -DMAILDIR="\"$(RSMTPSPOOL)"\"

TARMIN	= \
	rsmtp \
	bsmtp

BINMIN	= \
	$(BINDIR)/rsmtp \
	$(BINDIR)/bsmtp

# rules

all:	$(TARMIN)

install:	${BINDIR} \
		$(SPOOLDIR)/rsmtp \
		$(TARMIN) \
		$(BINMIN)

uninstall:
	-rm -f $(BINMIN) $(BINDIR)/rgsmtp $(BINDIR)/rcsmtp

clean:
	-rm -f $(CLEANFILES) $(TARMIN)

distclean:	clean

rsmtp:	rsmtp.o
	$(CC) -o rsmtp rsmtp.o $(LIBS) $(LDFLAGS)

bsmtp:	bsmtp.o
	$(CC) -o bsmtp bsmtp.o $(LIBS) $(LDFLAGS)

# install rules

$(BINDIR):
	mkdir -p ${BINDIR}

$(SPOOLDIR)/rsmtp:
	mkdir -p $(SPOOLDIR)/rsmtp
	chown $(USER) $(SPOOLDIR)/rsmtp
	chmod 1777 $(SPOOLDIR)/rsmtp

$(BINDIR)/rsmtp: rsmtp
	$(INSTALL_PROGRAM) rsmtp $@
	$(INSTALL_PROGRAM) rsmtp $(BINDIR)/rgsmtp
	$(INSTALL_PROGRAM) rsmtp $(BINDIR)/rcsmtp

$(BINDIR)/bsmtp: bsmtp
	$(INSTALL_PROGRAM) rsmtp $@

# end of makefile
# automatic depend
bsmtp.o: bsmtp.c ../include/istring.h ../include/sysexits2.h version.h
rsmtp.o: rsmtp.c ../include/istring.h ../include/iwait.h \
 ../include/sysexits2.h version.h
# eof
