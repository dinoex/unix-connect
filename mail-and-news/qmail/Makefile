# Makefile für das qmail-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

all: zbatchsmtp

clobber:
	-rm -f zbatchsmtp

zbatchsmtp: zbatchsmtp.in
	$(SHELL) ../../helpers/subst < zbatchsmtp.in > zbatchsmtp
