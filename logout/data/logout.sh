#!/bin/bash

# look for GDM/KDM/LDM/XDM
# if one of them is used, then it has launched either the session-manager, or the stand-alone script, both blocking.
# so just take this one, and kill it so that we return to GDM.
gdm_proc=`pgrep "gdm|kdm|ldm|xdm" | tail -1`
if test -n "$gdm_proc"; then
	last_process=`ps -ef | grep $gdm_proc | grep -v grep | tail -1 | tr -s " " | cut -d " " -f 2`
	if test -n "$last_process"; then
		kill $last_process
		exit 0
	fi
fi

# if the display manager couldn't be found, look for an endless sleep (likely to be the last process of the session), and kill it.
sleep_proc=`ps -ef | grep sleep | grep -v grep | head -1`
if test -n "$sleep_proc"; then
	sleep_time=`echo $sleep_proc | sed "s/.*sleep//g"`
	if test $sleep_time -gt 3600; then
		pkill sleep
	fi
fi
