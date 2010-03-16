
########### sources ###############

SET(cd-showDesktop_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_SHOW_DESKTOP} SHARED ${cd-showDesktop_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(show_desktopdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="icon.png")
add_definitions (-DMY_APPLET_CONF_FILE="showDesktop.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="showDesktop")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_SHOW_DESKTOP)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_SHOW_DESKTOP)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${SHOW_DESKTOP_XRANDR_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${SHOW_DESKTOP_XRANDR_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SHOW_DESKTOP}
	${PACKAGE_LIBRARIES}
	${SHOW_DESKTOP_XRANDR_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SHOW_DESKTOP} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-showDesktop.la
#
#
#if WITH_XRANDR_IN_SHOW_DESKTOP
#have_xrandr=-DHAVE_XRANDR="1"
#endif
#
#libcd_showDesktop_la_SOURCES = 
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
#libcd_showDesktop_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(SHOW_DESKTOP_XRANDR_CFLAGS) 
#	$(have_xrandr)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(show_desktopdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""icon.png""
#	-DMY_APPLET_CONF_FILE=""showDesktop.conf""
#	-DMY_APPLET_USER_DATA_DIR=""showDesktop""
#	-DMY_APPLET_VERSION=""$(VERSION_SHOW_DESKTOP)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_SHOW_DESKTOP)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_showDesktop_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#	$(SHOW_DESKTOP_XRANDR_LIBS)
#	
#
#
#libcd_showDesktop_la_LDFLAGS =
#	-avoid-version -module
