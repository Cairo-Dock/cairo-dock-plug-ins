
########### sources ###############

SET(cd-StatusNotifier_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_STATUS_NOTIFIER} SHARED ${cd-StatusNotifier_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(statusnotifierdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="StatusNotifier.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="StatusNotifier")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_STATUSNOTIFIER)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_STATUSNOTIFIER)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_STATUS_NOTIFIER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_STATUS_NOTIFIER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-StatusNotifier.la
#
#
#libcd_StatusNotifier_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_StatusNotifier_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(statusnotifierdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""StatusNotifier.conf""
#	-DMY_APPLET_USER_DATA_DIR=""StatusNotifier""
#	-DMY_APPLET_VERSION=""$(VERSION_STATUSNOTIFIER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_STATUSNOTIFIER)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_StatusNotifier_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_StatusNotifier_la_LDFLAGS =
#	-avoid-version -module
