# Makefile für etc bei unix-connect
# (c) Andreas Barth <aba@muenchen.pro-bahn.de> 1999
# steht unter der GPL

include ../Makefile.global

FILES=config approved alias autoeintrag.zconnect myself.zconnect

config: config.in ../Makefile.global
	sed "s=@ETCDIR@=$(ETCDIR)=; s=@BINDIR@=$(BINDIR)=; s=@SPOOLDIR@=$(SPOOLDIR)=" < config.in > config

all: config

install:
	for i in $(FILES); do \
	if test ! -f $(ETCDIR)/$i; then \
		install -o $(USER) -g $(GROUP) -m 660 $i $(ETCDIR); \
	fi; done

clean:

clobber:

tidy:
	-rm -f config config.in

