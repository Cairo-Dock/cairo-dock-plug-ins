#!/bin/bash

# This is a part of the external demo applet for Cairo-Dock
#
# Copyright : (C) 2010 by Nochka85
#                      modified by matttbe for the new API
#                      (based on the demo.py by Fabounet)
# E-mail : nochka85@glx-dock.org
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

### This is a part of the external applet demo_bash.py for cairo-dock
### Author : Nochka85
### Contact : nochka85@glx-dock.org
### Rev : 21/01/2010

DBUS_NAME="org.cairodock.CairoDock"
DBUS_PATH="/org/cairodock/CairoDock"
DBUS_INTERFACE="org.cairodock.CairoDock"
COMMAND=$0
SCRIPT_NAME=`basename $COMMAND`
APP_NAME="`echo $SCRIPT_NAME | cut -f1 -d '.' `"
ACTION=$1
DROP_DATA=$2
MENU_SELECT=$2
SCROLL_UP=$2
CONF_FILE="/home/$USER/.config/cairo-dock/current_theme/plug-ins/$APP_NAME/$APP_NAME.conf"

echo "bash initialized"

call()  # method, args
{
	echo "call $*"
	method=$1
	shift 1
	command=`echo "dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.$method $*"`
	echo "command: $command"
	eval $command
}

call_sub_icon()  # method, args
{
	echo "call_sub_icon $*"
	method=$1
	shift
	dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME/sub_icons "$DBUS_INTERFACE.$method" $*
}

get_conf_param()  # group, key
{
	LINE=`cat $CONF_FILE | grep "$1"`
	echo $LINE | cut -f2 -d '='
}

#############################################################################################################

on_click()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Left clic !"
}

on_middle_click()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Middle clic !"
}

on_scroll_icon()
{
	echo -n "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Scroll"
	if [ $1 -eq 1 ]; then
		echo " UP !"
	else
		echo " DOWN !"
	fi
}

on_drop_data()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> $1 has been dropped on applet !"
}

on_build_menu()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Build menu !"
}

on_menu_select()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Choice $1 has been selected !"
}

on_shortkey()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> key $1 has been pressed !"
}

on_change_focus()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> focus has changed to $1 !"
}

on_answer_dialog()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> $2 has been answered with button $1!"
}

on_click_sub_icon()
{
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> sub-icon $2 has been left-clicked !"
}

#############################################################################################################

begin()
{
	echo "start !"
}

end()
{
	echo "stop !"
}

reload()
{
	echo "reload !"
}

#############################################################################################################

run()
{
	echo "run $*"
	if [ "$1" = "on_click" ]; then
		on_click $2
	elif [ "$1" = "on_middle_click" ]; then
		on_middle_click
	elif [ "$1" = "on_scroll" ]; then
		on_scroll_icon $2
	elif [ "$1" = "on_drop_data" ]; then
		on_drop_data "$2"
	elif [ "$1" = "on_build_menu" ]; then
		on_build_menu
	elif [ "$1" = "on_menu_select" ]; then
		on_menu_select $2
	elif [ "$1" = "on_answer_dialog" ]; then
		on_answer_dialog $2 "$3"
	elif [ "$1" = "on_shortkey" ]; then
		on_shortkey "$2"
	elif [ "$1" = "on_change_focus" ]; then
		on_change_focus $2
	elif [ "$1" = "on_click_sub_icon" ]; then
		on_click_sub_icon $2 "$3"
	elif [ "$1" = "begin" ]; then
		begin
	elif [ "$1" = "end" ]; then
		end
	elif [ "$1" = "reload" ]; then
		reload
	fi
}
