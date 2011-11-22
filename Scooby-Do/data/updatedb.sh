#!/bin/bash

# UpdateDB for Cairo-Dock
#
# Copyright : (C) see the 'copyright' file.
# E-mail    : see the 'copyright' file.
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

export PRUNEPATHS="/tmp /usr /lib /var /bin /boot /sbin /etc /sys /proc /dev /root $HOME/.[^.]*"
updatedb --prunepaths="`echo $PRUNEPATHS`" -U / --output="$HOME/.config/cairo-dock/ScoobyDo/ScoobyDo.db" -l0
exit 0
