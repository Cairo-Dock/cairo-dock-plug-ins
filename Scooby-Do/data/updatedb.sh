#!/bin/bash
export PRUNEPATHS="/tmp /usr /lib /var /bin /boot /sbin /etc /sys /proc /dev /root $HOME/.[^.]*"
updatedb --prunepaths="`echo $PRUNEPATHS`" -U / --output="$HOME/.config/cairo-dock/ScoobyDo/ScoobyDo.db" -l0
exit 0
