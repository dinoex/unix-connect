# Generated automatically from Makefile.in by configure.
# Makefile für etc bei unix-connect
# (c) Andreas Barth <aba@muenchen.pro-bahn.de> 1999
# steht unter der GPL

include ../Makefile.global

FILES=config approved alias autoeintrag.zconnect myself.zconnect

all:

install:
	for i in $(FILES); do \
	if test ! -f $(ETCDIR)/$$i; then install -o $(USER) -g uucp -m 660 $$i $(ETCDIR); fi; done

