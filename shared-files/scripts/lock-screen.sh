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

if test -n "`ps -ef | grep gnome-screensaver | grep -v grep`"; then  ## "gnome-screensaver" is too long for pgrep
	gnome-screensaver-command --lock
elif test -n "`ps -ef | grep xscreensaver | grep -v grep`"; then
	xscreensaver-command -lock
else
	xlock
fi

exit 0
