#!/bin/bash

# This is a part of the external applets for Cairo-Dock
# Copyright : (C) 2010-2011 by Nochka85
#                      modified by matttbe for the new API
#                      (based on the demo.py by Fabounet)
# E-mail : fabounet@glx-dock.org, nochka85@glx-dock.org
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

## Rev : 09/02/2011

APP_NAME="$1"
shift 1
DBUS_NAME="org.cairodock.CairoDock"
DBUS_PATH="$1"
shift 1
DBUS_INTERFACE="org.cairodock.CairoDock"
CONF_FILE="$1"
shift 1
ROOT_DATA_DIR="$1"
shift 1
PARENT_APP_NAME="$1"
shift 1
SHARE_DATA_DIR="$PWD"

BOTTOM=0
TOP=1
RIGHT=2
LEFT=3
DOCK=0
DESKLET=1
EMBLEM_TOP_LEFT=0
EMBLEM_BOTTOM_RIGHT=1
EMBLEM_BOTTOM_LEFT=2
EMBLEM_TOP_RIGHT=3
EMBLEM_MIDDLE=4
EMBLEM_BOTTOM=5
EMBLEM_TOP=6
EMBLEM_RIGHT=7
EMBLEM_LEFT=8
MENU_ENTRY=0
MENU_SUB_MENU=1
MENU_SEPARATOR=2
MENU_CHECKBOX=3
MENU_RADIO_BUTTON=4
MAIN_MENU_ID=0
DIALOG_KEY_ENTER=-1
DIALOG_KEY_ESCAPE=-2

call() {  # method, args
	method=$1
	shift 1
	command=`echo -e "dbus-send --session --dest=$DBUS_NAME $DBUS_PATH $DBUS_INTERFACE.applet.$method $*"`
	eval $command
}

call_sub_icon() { # method, args
	method=$1
	shift
	command=`echo "dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/sub_icons $DBUS_INTERFACE.subapplet.$method $*"`
	eval $command
}

get_conf_param() { # group, key
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
	shift 4
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
