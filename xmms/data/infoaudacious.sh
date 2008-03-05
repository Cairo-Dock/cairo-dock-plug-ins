#!/bin/bash

FILE="/tmp/audacious-info_$USER.0"
rm $FILE

STATUS=$(audtool playback-status)

if [  "$STATUS" = "audtool: audacious server is not running!" ]
then
echo ""

else
echo "Audacious Emulated pipe" >> $FILE
echo "Pipe created by ChAnGFu" >> $FILE
#Status du player
echo "status: $(audtool playback-status)" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
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
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Titre du son
echo "nowTitle: $(audtool current-song)" >> $FILE
fi
