#!/bin/sh
dbus-binding-tool --prefix=_ --mode=glib-client  --output=messages-service-client.h messages-service.xml
