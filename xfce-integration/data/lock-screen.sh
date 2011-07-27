#!/bin/bash

if test -n "`ps -ef | grep gnome-screensaver`"; then
	gnome-screensaver-command --lock
elif test -n "`ps -ef | grep xscreensaver`"; then
	xscreensaver-command -lock
else
	xlock
fi

exit 0
