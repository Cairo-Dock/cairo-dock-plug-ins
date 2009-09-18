#!/bin/bash
#Auteur : necropotame
#Contact : adrien.pilleboue@gmail.com
#Version : 2008-03-12

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_main ./src/dbus_introspectable.xml > ./src/dbus-main-spec.h

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_applet ./src/dbus_applet_introspectable.xml > ./src/dbus-applet-spec.h

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_sub_applet ./src/dbus_sub_applet_introspectable.xml > ./src/dbus-sub-applet-spec.h
