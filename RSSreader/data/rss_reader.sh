#!/bin/bash  

# This is a part of the RSSreader applet Cairo-Dock
# (based on the RSS Display Script by Bill Woodford (admin@sdesign.us))
#
# Copyright : (C) 2009 by Yann Dulieu (Nochka85)
# E-mail : nochka85@cairo-dock.org
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL
#
#
# Rev : 0.0.4
# Date : 09/09/20


URL=$1
LINES=$2
TITLE_NUM=$3

#############################################################################################################
# START ### DO NOT CHANGE THIS SECTION
#############################################################################################################

if [[ "$URL" == "" ]]; then
    echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> No URL specified !" >&2
else
	### Set defaults values if none specified
	if [[ $LINES == "" ]]; then
		LINES=5
	fi

	if [[ $TITLE_NUM == "" ]]; then
		TITLE_NUM=1
	fi

	curl -s --connect-timeout 300 $URL |\
	sed "s/<\/title>/\n/g ; s/<\/description>/\n/g ; s/<\/link>/\n/g" |\
	sed "s/<title>/\n<title>/g ; s/<description>/\n<description>/g ; s/<link>/\n<link>/g" |\
	grep -E "title|description|link" |\
	sed -e 's/<\!\[CDATA\[//' |\
	sed -e 's/\]\]>//' |\
	head -n $((3*$LINES+3))
fi

exit



