
########### sources ###############

SET (cd-Dbus_LIB_SRCS
	applet-init.c 			applet-init.h
	applet-config.c 			applet-config.h
	applet-dbus.c 			applet-dbus.h
	interface-main-methods.c 	interface-main-methods.h
	interface-applet-object.c 	interface-applet-object.h
	interface-applet-methods.c 	interface-applet-methods.h
	interface-applet-signals.c 	interface-applet-signals.h
	dbus-applet-spec.h
	dbus-sub-applet-spec.h
	dbus-main-spec.h
	applet-struct.h
)

add_library (${PACKAGE_DBUS} SHARED ${cd-Dbus_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${dbusdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="Dbus.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Dbus")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DBUS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DBUS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DBUS}
	${PACKAGE_LIBRARIES})

########### install files ###############

install (TARGETS ${PACKAGE_DBUS} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-Dbus.la
#
#
#libcd_Dbus_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-dbus.c
#	applet-dbus.h
#	interface-main-methods.c
#	interface-main-methods.h
#	interface-applet-object.c
#	interface-applet-object.h
#	interface-applet-methods.c
#	interface-applet-methods.h
#	interface-applet-signals.c
#	interface-applet-signals.h
#	dbus-applet-spec.h
#	dbus-sub-applet-spec.h
#	dbus-main-spec.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_Dbus_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${dbusdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""none""
#	-DMY_APPLET_CONF_FILE=""Dbus.conf""
#	-DMY_APPLET_USER_DATA_DIR=""Dbus""
#	-DMY_APPLET_VERSION=""${VERSION_DBUS)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_DBUS)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_Dbus_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_Dbus_la_LDFLAGS =
#	-avoid-version -module
