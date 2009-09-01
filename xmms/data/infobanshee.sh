#!/bin/bash

# Compiz check for Cairo-Dock
#
# Copyright : (C) 2009 by Rémy Robertson
# E-mail    : fabounet@users.berlios.de
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

# Banshee Emulated pipe
# Pipe created by ChAnGFu


STATUS=$(banshee --query-status)
STATUS=${STATUS#*:}

if [  "$STATUS" -eq "-1" ]; then
  exit
fi

TITLE=$(banshee --query-title)
ARTIST=$(banshee --query-artist)
GTITLE="${ARTIST#*:} -${TITLE#*:}"
TTIMESEC=$(banshee --query-duration)
TTIMESEC=${TTIMESEC#*:}
POSITIONSEC=$(banshee --query-position)
POSITIONSEC=${POSITIONSEC#*:}

M=$(($TTIMESEC/60))
S=$(($TTIMESEC%60))
TTIME="$M:$S"

M=$(($POSITIONSEC/60))
S=$(($POSITIONSEC%60))
ETIME="$M:$S"

#Status du player
if [  "$STATUS" -eq "1" ]; then
  echo "status: Playing"
elif [  "$STATUS" -eq "0" ]; then
  echo "status: Paused"
fi
#Position du morceaux
echo "trackInPlaylist: N/A"
#Position actuelle en secondes
echo "uSecPosition: $POSITIONSEC"
#Temps écoulé
#echo "timeElapsed $ETIME"
echo ""
#Temps total en secondes
echo "totalTimeInSec $TTIMESEC"
#Temps total du son
#echo "totalTime: $TTIME"
echo ""
#Titre du son
echo "nowTitle: $GTITLE"
