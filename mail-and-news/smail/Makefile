# Makefile fuer das smail-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

all: zbatchsmtp

distclean:
	-rm -f zbatchsmtp

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > zbatchsmtp
