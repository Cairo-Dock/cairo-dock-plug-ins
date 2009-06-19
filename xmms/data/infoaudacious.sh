#!/bin/bash
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
