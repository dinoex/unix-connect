# Makefile fuer das qmail-Subverzeichnis

include ../../Makefile.global

FILES = zbatchsmtp

all: $(FILES)

install: $(FILES)
	$(INSTALL_PROGRAM) $(FILES) $(BINDIR)
	
uninstall:
	-cd $(BINDIR) && rm -f $(FILES)

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f $(FILES)

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > zbatchsmtp

