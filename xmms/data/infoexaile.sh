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

# Exaile Emulated pipe
# Pipe created by ChAnGFu

EXAILE=`ps aux | grep exaile | grep -v grep | grep -v info`
if [ "x$EXAILE" = "x" ]; then
  exit
fi

TEST=$(exaile -q)
if  [ "$TEST" = "No track playing" ]; then
  exit
fi

STATUS=${TEST#status: *}
STATUS=${STATUS%self:*}

TITLE=${TEST%artist:*}
TITLE=${TITLE#*self: }

ARTIST=${TEST% album:*}
ARTIST=${ARTIST#*artist: }
GTITLE="$ARTIST - $TITLE"

TTIME=${TEST%position:*}
TTIME=${TTIME#*length: }

POSITION=${TEST#*position: }
POSITION=${POSITION#*[}
POSITION=${POSITION%*]}

#Status du player
echo "status: $STATUS"
#Position du morceaux
echo "trackInPlaylist: N/A"
#Position actuelle en secondes
echo "uSecPosition: N/A"
#Temps écoulé
echo "timeElapsed $POSITION"
#Temps total en secondes
echo "totalTimeInSec N/A"
#Temps total du son
echo "totalTime: $TTIME"
#Titre du son
echo "nowTitle: $GTITLE"
