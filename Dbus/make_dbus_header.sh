#!/bin/bash
#Auteur : necropotame
#Contact : adrien.pilleboue@gmail.com
#Version : 2008-03-12

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_callback ./src/dbus_introspectable.xml > ./src/cairo-dock-dbus-spec.h

