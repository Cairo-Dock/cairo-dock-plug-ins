#!/bin/bash
#
# Copyright : (C) see the 'copyright' file.
# E-mail    : see the 'copyright' file.
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

### d'après un script posté par naholyr sur ubuntu-fr.org, adapte pour Cairo-Dock.

TMP=$(tempfile)
curl -q -v -T "$1" --limit-rate $2 -u cairo@dock.org:toto ftp://dl.free.fr/ 2> "$TMP"

if test $? -eq 0; then
	URL=$(grep -i -F "Il est disponible via" $TMP | grep -o "http:[^ ]*")
	if test "x$URL" != "x" ; then
		echo $URL
	fi
fi
#rm -f "$TMP"
exit 0
