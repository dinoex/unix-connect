# Makefile fuer das inn-Subverzeichnis

include ../../Makefile.global

FILES = send-zcon 

all: $(FILES)

install: $(FILES)
	$(INSTALL_PROGRAM) $(FILES) $(BINDIR)

uninstall:
	-cd $(BINDIR) && rm -f $(FILES)

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f $(FILES)

send-zcon: send-zcon.in
	$(SHELL) ../../helpers/subst < send-zcon.in > send-zcon

