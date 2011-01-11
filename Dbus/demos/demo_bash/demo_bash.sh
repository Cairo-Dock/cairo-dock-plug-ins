#!/bin/bash

# This is a part of the external demo applet for Cairo-Dock
#
# Copyright : (C) 2010-2011 by Nochka85
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

. /usr/share/cairo-dock/plug-ins/Dbus/CDBashApplet.sh

#############################################################################################################

get_ALL_conf_params()
{
	demo_text=`get_conf_param "demo_text"`
	
	demo_truefalse=`get_conf_param "demo_truefalse"`
	
	demo_value=`get_conf_param "demo_value"`
}

#############################################################################################################

on_click()
{
	call ShowDialog "string:\"Left clic\"" "int32:2"
}

on_middle_click()
{
	call ShowDialog "string:\"Middle clic\"" "int32:2"
}


on_scroll_icon()
{
	i=0
	if [ -f store ]; then
		i=`cat store`
	fi
	if [ $1 -eq 1 ]; then
		let "i += 1"
	else
		let "i -= 1"
	fi
	call SetQuickInfo "string:$i"
	echo "$i" > store
}

on_drop_data()
{
	if [ "`echo $1 |grep file://`" ]; then # It's a file !
		file="`echo $1 | cut -c 8-`" # we remove 'file://' before the location
		call ShowDialog "string:\"FILE : $file has been dropped on the applet\"" "int32:5"
	else	# It's an URL !
		call ShowDialog "string:\"URL : $1 has been dropped on the applet\"" "int32:5"  # we keep the 'http://' in the name
	fi
}

on_build_menu()
{
	call PopulateMenu "array:string:\"choice 0\",\"choice 1\""
}

on_menu_select()
{
	call ShowDialog "string:\"Choice $1 has been selected\"" "int32:4"
}

#############################################################################################################

begin()
{
	get_ALL_conf_params
	
	echo "from file $CONF_FILE:"
	echo "The demo_text in config is : $demo_text"
	echo "The demo_truefalse in config is : $demo_truefalse"
	echo "The demo_value in config is : $demo_value"
	
	echo "Our module is started"
	call ShowDialog "string:\"I'm connected to Cairo-Dock \!\"" "int32:4"
	call SetQuickInfo "string:0"
	
	echo "0" > store
}

end()
{
	rm -f store
}

reload()
{
	get_ALL_conf_params
	
	echo "The demo_text in config is : $demo_text"
	echo "The demo_truefalse in config is : $demo_truefalse"
	echo "The demo_value in config is : $demo_value"
	
	echo "Our module is reloaded"
	call ShowDialog "string:\"Our module is reloaded\"" "int32:2"
}

#############################################################################################################

echo "our module is called with $*"
run $*

exit 0
