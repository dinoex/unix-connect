# Dummy-Makefile für kein Mail- bzw. kein Newssystem zu konfigurieren

include ../../Makefile.global

FILES = viazconnect

all: $(FILES)

install:

uninstall:

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f $(FILES)

viazconnect: viazconnect.in
	$(SHELL) ../../helpers/subst < viazconnect.in > $@

