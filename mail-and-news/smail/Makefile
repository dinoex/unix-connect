# Makefile für das smail-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

all: zbatchsmtp

distclean:
	-rm -f zbatchsmtp

zbatchsmtp: zbatchsmtp.in
	../../helpers/subst < zbatchsmtp.in > zbatchsmtp
