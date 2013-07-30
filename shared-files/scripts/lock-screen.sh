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

# Notes:
# * "gnome-screensaver" is too long for pgrep
# * PS_OUTPUT will contain only one big line

PS_OUTPUT=`ps -u $USER -wwo pid,cmd` # restricted to the current user
if test -n "`echo $PS_OUTPUT | grep gnome-screensaver`"; then
	gnome-screensaver-command --lock
elif test -n "`echo $PS_OUTPUT | grep xscreensaver`"; then
	xscreensaver-command -lock
elif test -n "`echo $PS_OUTPUT | grep cinnamon-screensaver`"; then
	cinnamon-screensaver-command --lock
elif test -n "`echo $PS_OUTPUT | grep light-locker`"; then
	light-locker-command --lock
elif hash xlock 2> /dev/null; then
	xlock
else # check is another "*-screensaver" daemon is running
	# we need to relaunch ps, easier to parse compare to PS_OUTPUT which contains only one big line
	SCREENSAVER=`ps -u $USER -wwo pid,cmd | grep "\-[s]creensaver" | awk '{print $2}'`
	if test -n "$SCREENSAVER"; then
		if hash ${SCREENSAVER}-command 2> /dev/null; then
			${SCREENSAVER}-command --lock
		else
			echo "WARNING: ${SCREENSAVER} is running but ${SCREENSAVER}-command is not available."
			echo "Please report this bug to Cairo-Dock devs on http://forum.glx-dock.org"
			exit 1
		fi
	else
		echo "WARNING: No screensaver found! Please report this bug to Cairo-Dock devs on http://forum.glx-dock.org"
		exit 1
	fi
fi
# the return value is given by the last program launched
