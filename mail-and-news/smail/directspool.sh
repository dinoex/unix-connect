#!/bin/sh
#
#------------------------------------------------------------------------------
#  ZCONNECT fuer UNIX
#
#  Dieses Shell-Script liest Daten aus einem NFS-Verzeichnis per ZCONNECT
#  ein, wandelt sie in Batched-SMTP bzw. RNews-Batche um und verfuettert
#  sie an die entsprechenden UNIX-Programme.
#
#  Das ganze wird aus der root-Crontab regelmaessig aufgerufen.
#
#  Directory-Struktur dazu: hier (Sys V R4) liegt alles unter /var/spool/...
#
#  Es gibt dann:   /var/spool/direct/smtp                 fuer Mail
#                  /var/spool/direct/news                 fuer News
#
#  Darunter jeweils ein Directory mit dem Rechner-Namen, wo die Daten hin
#  sollen: also z.B.  /var/spool/direct/news/sisyphos   sind News, die zu
#  sisyphos gehen (aber noch im ZCONNECT-Format vorliegen!)
#
#------------------------------------------------------------------------------
#

cd /var/spool/direct

for file in smtp/sisyphos/*
do
	if [ "$file" != 'smtp/sisyphos/*' ]
	then
		mv $file /tmp/dirsp.$$
		/usr/local/lib/zconnect/uuwsmtp -f /tmp/dirsp.$$ | rsmtp
		rm -f /tmp/dirsp.$$
	fi
done

for file in news/sisyphos/*
do
	if [ "$file" != 'news/sisyphos/*' ]
	then
		mv $file /tmp/dirsp.$$
		/usr/local/lib/zconnect/uuwnews -f /tmp/dirsp.$$ | rnews
		rm -f /tmp/dirsp.$$
	fi
done
