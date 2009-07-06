#!/bin/bash
### d'après un script posté par naholyr sur ubuntu-fr.org, adapte pour Cairo-Dock.

TMP=$(tempfile)
curl -q -v -T "$1" -u cairo@dock.org:toto ftp://dl.free.fr/ 2> "$TMP"

if test $? -eq 0; then
	URL=$(grep -i -F "Il est disponible via" $TMP | grep -o "http:[^ ]*")
	if test "x$URL" != "x" ; then
		echo $URL
	fi
fi
#rm -f "$TMP"
exit 0
