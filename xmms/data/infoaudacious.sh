#!/bin/bash
# Audacious Emulated pipe
# Pipe created by ChAnGFu

FILE="/tmp/audacious-info_$USER.0"

STATUS=$(audtool playback-status)
TITLE=$(audtool current-song)

if [  "$STATUS" = "audtool: audacious server is not running!" ]; then
  exit
fi
if [  "$TITLE" = "No song playing." ]; then
  exit
fi

#Status du player
echo "status: $STATUS" >> $FILE
#Position du morceaux
echo "trackInPlaylist: $(audtool playlist-position)" >> $FILE
#Position acutel en seconde 
echo "uSecPosition: $(audtool current-song-output-length-frames)" >> $FILE
#Temps écoulé
echo "timeElapsed $(audtool current-song-output-length)" >> $FILE
#Temps total en seconde
echo "totalTimeInSec $(audtool current-song-length-frames)" >> $FILE
#Temps total du son
echo "totalTime: $(audtool current-song-length)" >> $FILE
#Titre du son
echo "nowTitle: $TITLE" >> $FILE
