
########### sources ###############

SET(cd-weblets_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-widget.c
	applet-widget.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_WEBLETS} SHARED ${cd-weblets_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(webletsdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="weblets.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="weblets")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_WEBLETS)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_WEBLETS)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE=1)

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${WEBKIT_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${WEBKIT_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_WEBLETS}
	${PACKAGE_LIBRARIES}
	${WEBKIT_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_WEBLETS} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-weblets.la
#
#
#libcd_weblets_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-widget.c
#	applet-widget.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_weblets_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS) $(WEBKIT_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(webletsdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""weblets.conf""
#	-DMY_APPLET_USER_DATA_DIR=""weblets""
#	-DMY_APPLET_VERSION=""$(VERSION_WEBLETS)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_WEBLETS)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE=1
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#libcd_weblets_la_LIBADD = 
#	$(PACKAGE_LIBS) $(WEBKIT_LIBS) -lm
#
#
#libcd_weblets_la_LDFLAGS =
#	-avoid-version -module
