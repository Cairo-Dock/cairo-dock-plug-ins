#!/bin/bash
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

PS_OUTPUT=`ps -u $USER -wwo pid,cmd` # restricted to the current user
if test -n "`echo $PS_OUTPUT | grep gnome-screensaver`"; then  ## "gnome-screensaver" is too long for pgrep
	gnome-screensaver-command --lock
elif test -n "`echo $PS_OUTPUT | grep xscreensaver`"; then
	xscreensaver-command -lock
elif test -n "`echo $PS_OUTPUT | grep cinnamon-screensaver`"; then
	cinnamon-screensaver-command --lock
else
	xlock
fi

exit 0
