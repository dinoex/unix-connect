# Makefile fuer das inn-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

all: send-zcon

distclean:
	-rm -f send-zcon

send-zcon: send-zcon.in
	$(SHELL) ../../helpers/subst < send-zcon.in > send-zcon
