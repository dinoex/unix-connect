# Makefile fuer das Postfix-Subverzeichnis

include ../../Makefile.global

FILES = zbatchsmtp run_as

all: $(FILES) README ucpf.pl

install: $(FILES)
	$(INSTALL_PROGRAM) $(FILES) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(FILES)

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f $(FILES) README ucpf.pl

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > $@

README: README.in
	$(SHELL) ../../helpers/subst < README.in > $@

ucpf.pl: ucpf.pl.in
	$(SHELL) ../../helpers/subst < ucpf.pl.in > $@

run_as: run_as.c
