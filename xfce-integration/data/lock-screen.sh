#!/bin/bash

if test -n "`ps -ef | grep gnome-screensaver | grep -v grep`"; then  ## "gnome-screensaver" is too long for pgrep
	gnome-screensaver-command --lock
elif test -n "`ps -ef | grep xscreensaver | grep -v grep`"; then
	xscreensaver-command -lock
else
	xlock
fi

exit 0
