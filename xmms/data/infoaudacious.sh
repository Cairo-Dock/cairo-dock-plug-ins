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

# Audacious Emulated pipe
# Pipe created by ChAnGFu

STATUS=$(audtool playback-status)
TITLE=$(audtool current-song)

if [  "$STATUS" = "audtool: audacious server is not running!" ]; then
  exit
fi
if [  "$TITLE" = "No song playing." ]; then
  exit
fi

#Status du player
echo "status: $STATUS"
#Position du morceaux
echo "trackInPlaylist: $(audtool playlist-position)"
#Position actuelle en secondes
echo "uSecPosition: $(audtool current-song-output-length-frames)"
#Temps écoulé
#echo "timeElapsed $(audtool current-song-output-length)"
echo ""
#Temps total en secondes
echo "totalTimeInSec $(audtool current-song-length-frames)"
#Temps total du son
#echo "totalTime: $(audtool current-song-length)"
echo ""
#Titre du son
echo "nowTitle: $TITLE"
