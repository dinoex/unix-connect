# Makefile fuer etc bei unix-connect
# (c) Andreas Barth <aba@muenchen.pro-bahn.de> 1999
# steht unter der GPL

include ../Makefile.global

FILES=	config approved alias autoeintrag.zconnect myself.zconnect \
	systems.sample zconnecthosts xprogs

all: config xprogs

config: config.in ../Makefile.global
	$(SHELL) ../helpers/subst < config.in > config

xprogs: xprogs.in ../Makefile.global
	$(SHELL) ../helpers/subst < xprogs.in > xprogs

install:
	for i in $(FILES); do \
	if test ! -f $(ETCDIR)/$$i; then \
		$(INSTALL_DATA) $$i $(ETCDIR); \
	fi; done

uninstall:

clean:
	-rm -f $(CLEANFILES)

distclean:	clean
	-rm -f config xprogs

