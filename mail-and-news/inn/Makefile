# Makefile fuer das inn-Subverzeichnis

include ../../Makefile.global
include ../../Makefile.includes

FILES = send-zcon 

all: $(FILES)

install: $(FILES)
	$(INSTALL_PROGRAM) $(FILES) $(BINDIR)

distclean:
	-rm -f send-zcon

send-zcon: send-zcon.in
	$(SHELL) ../../helpers/subst < send-zcon.in > send-zcon
