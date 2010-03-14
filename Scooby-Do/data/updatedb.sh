#!/bin/bash

# Compiz check for Cairo-Dock
#
# Copyright : (C) 2009 by Rémy Robertson
# E-mail    : fabounet@glx-dock.org
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

export PRUNEPATHS="/tmp /usr /lib /var /bin /boot /sbin /etc /sys /proc /dev /root $HOME/.[^.]*"
updatedb --prunepaths="`echo $PRUNEPATHS`" -U / --output="$HOME/.config/cairo-dock/ScoobyDo/ScoobyDo.db" -l0
exit 0
