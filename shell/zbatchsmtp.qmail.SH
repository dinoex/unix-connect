#!/bin/sh
# $Id$

# deliver messages accumlated into subdirectories of the
# z-outq spool directory.  Subdirectory names are based on
# the actual hostnames involved (including full domain!)

LOCK=@SPOOLDIR@/locks/zbatchsmtp.lck
ERRDIR=@SPOOLDIR@/errors

trap "rm -f $LOCK.$$; exit 0" 0 1 2 3

echo $$ > $LOCK.$$
if (umask 222; ln $LOCK.$$ $LOCK) 2>/dev/null
then
	# OK -> Datei gab es noch nicht
	trap "rm -f $LOCK $LOCK.$$; exit 0" 0 1 2 3 
	rm -f $LOCK.$$
else
	# Fehler -> Lock existierte
	echo "$0: es laeuft bereits ein anderer Prozess" >&2
	ls -l $LOCK >&2
	exit 0
fi

LOCALHOST=@FQDN@
MAXSIZE=100000

MAILSPOOL=/var/spool/zconnect
PATH=$PATH:/usr/local/bin:/usr/lib/news:/usr/local/lib/news/bin:/usr/local/lib/zconnect:/var/qmail/bin; export PATH


maildir2bsmtp -s100000 /var/spool/zconnect/sys/bionic \
	"alias-bionic-" "uursmtp -f bionic.zerberus.de"

