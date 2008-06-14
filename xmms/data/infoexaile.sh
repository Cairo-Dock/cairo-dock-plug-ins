#!/bin/bash
# Exaile Emulated pipe
# Pipe created by ChAnGFu

if [ ! -w $1 ] ; then exit 1 ; fi
FILE=$1

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
echo "status: $STATUS" > $FILE
#Position du morceaux
echo "trackInPlaylist: N/A" >> $FILE
#Position actuelle en secondes
echo "uSecPosition: N/A" >> $FILE
#Temps écoulé
echo "timeElapsed $POSITION" >> $FILE
#Temps total en secondes
echo "totalTimeInSec N/A" >> $FILE
#Temps total du son
echo "totalTime: $TTIME" >> $FILE
#Titre du son
echo "nowTitle: $GTITLE" >> $FILE
