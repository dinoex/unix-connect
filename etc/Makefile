# Makefile für etc bei unix-connect
# (c) Andreas Barth <aba@muenchen.pro-bahn.de> 1999
# steht unter der GPL

include ../Makefile.global
include ../Makefile.includes

FILES=config approved alias autoeintrag.zconnect myself.zconnect

all: config

config: config.in ../Makefile.global
	$(SHELL) ../helpers/subst < config.in > config

install:
	for i in $(FILES); do \
	if test ! -f $(ETCDIR)/$i; then \
		install -o $(USER) -g $(GROUP) -m 660 $i $(ETCDIR); \
	fi; done

tidy:
	-rm -f config

