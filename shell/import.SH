#!/bin/sh
# $Id$
#
#  import (Netzwerk-System) (Datenformat)
#
#  Dieses Shell-Script wird von den Online-Programmen nach Austausch der
#  Daten aufgerufen. Im aktuellen Verzeichnis befinden sich die bereits
#  ausgepackten Daten (und sonst nichts). Diese muessen nun noch 
#  konvertiert und an das News/Mail-System weitergereicht werden.

IAM=@FQDN@

#
#  Wichtig: hier den Pfad so erweitern, das die Gateway-Programme sowie
#  smail und rnews erreichbar sind!
#
PATH=@DESTDIR@:@PATHADD@:$PATH; export PATH

#
#  Der Smail-Aufruf zum Einlesen der Mails (nach der Konvertierung in
#  Batched-SMTP Format)
#
RSMTP="smail -bm -bS -obq -oMs $1 -oMr zconnect"

echo '##################################################################' >&2
date >&2
echo "Import der empfangenen Daten von $1" >&2

TMP=import.$$

if [ -d @SPOOLDIR@/error ]
then
	echo >/dev/null
else
	mkdir @SPOOLDIR@/error || { echo "Ich kann @SPOOLDIR@/error nicht anlegen!"; exit 1; }
fi

mkdir $TMP || { echo "kann temporaer-Verzeichnis nicht anlegen!"; exit 1; }

for file in *
do
	if [ -f $file ]
	then
		echo "Lese $file" >&2

		mv $file $TMP
		cd $TMP
		zconsplit $file
		if [ -f $file.prv ]
		then
			(((uuwsmtp -f $file.prv $1 | $RSMTP) 2>> /tmp/zlog)||(mv $file.prv @SPOOLDIR@/error/; echo $file.prv " defekt -> @SPOOLDIR@/error/" |mail postmaster))
		fi
		if [ -f $file.brt ]
		then
			(((uuwnews -f $file.brt $1 | rnews) 2>> /tmp/zlog)||(mv $file.brt @SPOOLDIR@/error/; echo $file.brt " defekt -> @SPOOLDIR@/error/" |mail postmaster))
		fi
		if [ -f $file ]
		then
			rm -f $file
		fi
		cd ..
	fi
done

rm -rf $TMP

# $TMP haengt von unserer Prozess-ID ab, kann also nicht belegt sein.
