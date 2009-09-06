#!/bin/bash

### This is a part of the external applet demo_bash.py for cairo-dock
### Author : Nochka85
### Contact : nochka85@cairo-dock.org
### Rev : 09/09/06

APP_FOLDER=$(pwd)

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



POSITION_IN_CONF="2"
DESCRIPTION="This is a distant applet in bash by Nochka85"


#############################################################################################################
register_the_applet() {
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH $DBUS_INTERFACE.RegisterNewModule string:"$APP_NAME" int32:$POSITION_IN_CONF string:"$DESCRIPTION" string:"$APP_FOLDER"
exit
}

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
get_ALL_conf_params

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Left clic !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Left clic !" int32:2

exit
}

#############################################################################################################
action_on_middle_click() {
get_ALL_conf_params

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Middle clic !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.ShowDialog string:"Middle clic !" int32:2

exit
}

#############################################################################################################
action_on_scroll_icon() {
get_ALL_conf_params

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
get_ALL_conf_params

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
get_ALL_conf_params

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
get_ALL_conf_params

echo "$APP_NAME applet -> Script Name : $SCRIPT_NAME -> Build menu !"
dbus-send --session --dest=$DBUS_NAME $DBUS_PATH/$APP_NAME $DBUS_INTERFACE.applet.PopulateMenu array:string:"choice 0","choice 1"

exit
}

#############################################################################################################
action_on_menu_select() {
get_ALL_conf_params

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

