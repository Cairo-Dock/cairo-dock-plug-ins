#!/bin/bash
# Banshee Emulated pipe
# Pipe created by ChAnGFu

FILE="/tmp/banshee-info_$USER.0"

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
  echo "status: Playing" > $FILE
elif [  "$STATUS" -eq "0" ]; then
  echo "status: Paused" > $FILE
fi
#Position du morceaux
echo "trackInPlaylist: N/A" >> $FILE
#Position actuelle en secondes
echo "uSecPosition: $POSITIONSEC" >> $FILE
#Temps écoulé
#echo "timeElapsed $ETIME" >> $FILE
echo "" >> $FILE
#Temps total en secondes
echo "totalTimeInSec $TTIMESEC" >> $FILE
#Temps total du son
#echo "totalTime: $TTIME" >> $FILE
echo "" >> $FILE
#Titre du son
echo "nowTitle: $GTITLE" >> $FILE
