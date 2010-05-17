#!/bin/sh
dbus-binding-tool --prefix=_ --mode=glib-client  --output=me-service-client.h me-service.xml
