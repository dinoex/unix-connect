# Makefile für das sendmail-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

all: zbatchsmtp RSMTP

distclean:
	-rm -f zbatchsmtp RSMTP

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > $@

RSMTP: RSMTP.in
	$(SHELL) ../../helpers/subst < RSMTP.in > $@
