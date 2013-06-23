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
#
#
# look for GDM/KDM/LDM/XDM/lightdm
# if one of them is used, then it has launched either the session-manager, or the stand-alone script, both blocking.
# so just take this one, and kill it so that we return to GDM.

gdm_proc=`pgrep "gdm|kdm|ldm|xdm|lightdm" | tail -1`
if test -n "$gdm_proc"; then
	last_process=`ps -ef | grep $gdm_proc | grep $USER | grep -v grep | tail -1 | tr -s " " | cut -d " " -f 2`
	if test -n "$last_process"; then
		kill $last_process
		exit 0
	fi
fi

# if the display manager couldn't be found, look for an endless sleep (likely to be the last process of the session), and kill it.
sleep_proc=`ps -ef | grep sleep | grep $USER | grep -v grep | head -1`
if test -n "$sleep_proc"; then
	sleep_time=`echo $sleep_proc | sed "s/.*sleep//g"`
	if test $sleep_time -gt 3600; then
		pkill sleep
	fi
fi
