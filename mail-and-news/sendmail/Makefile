# Makefile für das sendmail-Subverzeichnis

include ../../Makefile.global

FILES = zbatchsmtp mail.uuconnect

all: RSMTP uconnect.m4 $(FILES)

install: $(FILES)
	$(INSTALL_PROGRAM) $(FILES) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(FILES)

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f RSMTP uconnect.m4 $(FILES)

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > $@

RSMTP: RSMTP.in
	$(SHELL) ../../helpers/subst < RSMTP.in > $@

uconnect.m4: uconnect.m4.in
	$(SHELL) ../../helpers/subst < uconnect.m4.in > $@

mail.uuconnect: mail.uuconnect.in
	$(SHELL) ../../helpers/subst < mail.uuconnect.in > $@

