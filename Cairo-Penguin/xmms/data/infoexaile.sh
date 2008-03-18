#!/bin/bash
#!/bin/bash

FILE="/tmp/exaile-info_$USER.0"

EXAILE=`ps aux | grep exaile | grep -v grep | grep -v info`
if [ "$EXAILE" ]; then
  echo ""
else
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

echo "Exaile Emulated pipe" >> $FILE
echo "Pipe created by ChAnGFu" >> $FILE
#Status du player
echo "status: $STATUS" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Position du morceaux
echo "trackInPlaylist: N/A" >> $FILE
#Position acutel en seconde 
echo "uSecPosition: 0" >> $FILE
#Temps écoulé
echo "timeElapsed $POSITION" >> $FILE
#Temps total en seconde
echo "totalTimeInSec 0" >> $FILE
#Temps total du son
echo "totalTime: $TTIME" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Ligne non lu du pipe
echo "Info: Blank" >> $FILE
#Titre du son
echo "nowTitle: $GTITLE" >> $FILE
