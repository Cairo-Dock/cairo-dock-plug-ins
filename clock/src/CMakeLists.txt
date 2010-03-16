
########### sources ###############

SET(cd-clock_LIB_SRCS
applet-struct.h
applet-init.c
applet-init.h
applet-draw.c
applet-draw.h
applet-digital.c
applet-digital.h
applet-config.c
applet-config.h
applet-notifications.c
applet-notifications.h
applet-theme.c
applet-theme.h
)

add_library(${PACKAGE_CLOCK} SHARED ${cd-clock_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(clockdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="clock.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="$(clockuserdirname)")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_CLOCK)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_CLOCK)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_CLOCK}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_CLOCK} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-clock.la
#
#
#libcd_clock_la_SOURCES = 
#	applet-struct.h
#	applet-init.c
#	applet-init.h
#	applet-draw.c
#	applet-draw.h
#	applet-digital.c
#	applet-digital.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-theme.c
#	applet-theme.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_clock_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(clockdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""clock.conf""
#	-DMY_APPLET_USER_DATA_DIR=""$(clockuserdirname)""
#	-DMY_APPLET_VERSION=""$(VERSION_CLOCK)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_CLOCK)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	 -Wall
##	-g -ggdb -W -Wall
#
#
#libcd_clock_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_clock_la_LDFLAGS =
#	-avoid-version -module
