#!/bin/sh
# $Id$
#
#  import (Netzwerk-System) (Datenformat)
#
#  Dieses Shell-Script wird von den Online-Programmen nach Austausch der
#  Daten aufgerufen. Im aktuellen Verzeichnis befinden sich die bereits
#  ausgepackten Daten (und sonst nichts). Diese muessen nun noch 
#  konvertiert und an das News/Mail-System weitergereicht werden.

IAM=bi-neu.foebud.org

#
#  Wichtig: hier den Pfad so erweitern, das die Gateway-Programme sowie
#  qmail und rnews erreichbar sind!
#
PATH=/usr/local/lib/zconnect:/usr/local/bin:/usr/lib/news:/usr/local/lib/news/bin:/var/qmail/bin:$PATH; export PATH

#
#  Der Aufruf zum Einlesen der Mails (nach der Konvertierung in
#  Batched-SMTP Format)
#
RSMTP="/var/qmail/bin/rsmtp"

echo '##################################################################' >&2
date >&2
echo "Import der empfangenen Daten von $1" >&2

TMP=import.$$

if [ ! -d /var/spool/zconnect/error ]
then
	mkdir /var/spool/zconnect/error || { echo "Ich kann /var/spool/zconnect/error nicht anlegen!"; exit 1; }
fi

mkdir $TMP || { echo "kann temporaer-Verzeichnis nicht anlegen!"; exit 1; }

for file in *
do
	if [ -f $file ]
	then
		echo "Lese $file" >&2

		mv $file $TMP
		cd $TMP
		zconsplit $file || { echo "zconsplit $file failed: $?";  exit 1; }
		if [ -f $file.prv -a -s $file.prv ]
		then
			(((uuwsmtp -f $file.prv $1 | $RSMTP) 2>> /tmp/zlog)||(mv $file.prv /var/spool/zconnect/error/; echo $file.prv " defekt -> /var/spool/zconnect/error/" |mail postmaster))
		fi
		if [ -f $file.brt -a -s $file.brt ]
		then
			(((uuwnews -f $file.brt $1 | rnews) 2>> /tmp/zlog)||(mv $file.brt /var/spool/zconnect/error/; echo $file.brt " defekt -> /var/spool/zconnect/error/" |mail postmaster))
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


