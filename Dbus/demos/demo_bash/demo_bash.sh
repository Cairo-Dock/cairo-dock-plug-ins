#!/bin/bash

# This is a part of the external demo applet for Cairo-Dock
#
# Copyright : (C) 2010 by Nochka85
#                      modified by matttbe for the new API
#                      (based on the demo.py by Fabounet)
# E-mail : nochka85@cairo-dock.org
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
### Contact : nochka85@cairo-dock.org
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

#############################################################################################################
get_conf_param() {
LIGNE=`cat $CONF_FILE | grep "$1"`
PARAM="`echo $LIGNE | cut -f2 -d '=' `"
}

#############################################################################################################
get_ALL_conf_params() {

get_conf_param "demo_text"
demo_text="$PARAM"

get_conf_param "demo_truefalse"
demo_truefalse="$PARAM"

get_conf_param "demo_value"
demo_value="$PARAM"

}

#############################################################################################################
action_on_click() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Left clic !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Left clic !" int32:2

exit
}

#############################################################################################################
action_on_middle_click() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Middle clic !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Middle clic !" int32:2

exit
}

#############################################################################################################
action_on_scroll_icon() {
if [ $SCROLL_UP -eq "0" ]; then
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Scroll UP !"
	dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Scroll UP!" int32:1
else
	echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Scroll DOWN !"
	dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Scroll DOWN!" int32:1
fi

exit
}

#############################################################################################################
action_on_drop_data() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> $DROP_DATA has been dropped on applet !"


if [ "`echo $DROP_DATA |grep 'file://'`" != "" ]; then 	# It's a file !
	DROP_DATA="`echo $DROP_DATA | cut -c 8-`"  # we remove 'file://' before the location
	dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"FILE : $DROP_DATA has been dropped on applet !" int32:4
else	# It's an URL !
	dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"URL : $DROP_DATA has been dropped on applet !" int32:4	# we keep the 'http://' in the name
fi

exit
}

#############################################################################################################
action_on_init() {
get_ALL_conf_params

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_text in config is : $demo_text"
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_truefalse in config is : $demo_truefalse"
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_value in config is : $demo_value"

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Our module is started"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"I'm connected to Cairo-Dock !" int32:4
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.SetQuickInfo string:"123"

exit
}

#############################################################################################################
action_on_stop() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Our module is stopped"

exit
}

#############################################################################################################
action_on_reload() {
get_ALL_conf_params

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_text in config is : $demo_text"
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_truefalse in config is : $demo_truefalse"
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> The demo_value in config is : $demo_value"

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Our module is reloaded"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Our module is reloaded" int32:2

exit
}

#############################################################################################################
action_on_build_menu() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Build menu !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.PopulateMenu array:string:"choice 0","choice 1"

exit
}

#############################################################################################################
action_on_menu_select() {
echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Choice $MENU_SELECT has been selected !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Choice $MENU_SELECT has been selected !" int32:4

exit
}

#############################################################################################################
# START ### DO NOT CHANGE THIS SECTION
#############################################################################################################

if [ "`echo $ACTION |grep 'register_the_applet'`" != "" ]; then
	register_the_applet
elif [ "`echo $ACTION |grep 'action_on_click'`" != "" ]; then
	action_on_click
elif [ "`echo $ACTION |grep 'action_on_middle_click'`" != "" ]; then
	action_on_middle_click
elif [ "`echo $ACTION |grep 'action_on_scroll_icon'`" != "" ]; then
	action_on_scroll_icon
elif [ "`echo $ACTION |grep 'action_on_drop_data'`" != "" ]; then
	action_on_drop_data
elif [ "`echo $ACTION |grep 'action_on_init'`" != "" ]; then
	action_on_init
elif [ "`echo $ACTION |grep 'action_on_stop'`" != "" ]; then
	action_on_stop
elif [ "`echo $ACTION |grep 'action_on_reload'`" != "" ]; then
	action_on_reload
elif [ "`echo $ACTION |grep 'action_on_build_menu'`" != "" ]; then
	action_on_build_menu
elif [ "`echo $ACTION |grep 'action_on_menu_select'`" != "" ]; then
	action_on_menu_select
fi

exit

