
########### sources ###############

SET(cd-Toons_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-theme.c
	applet-theme.h
	applet-draw.c
	applet-draw.h
	applet-struct.h
)

add_library(${PACKAGE_TOONS} SHARED ${cd-Toons_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(toonsdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Toons.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Toons")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_TOONS)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_TOONS)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_TOONS}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_TOONS} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-Toons.la
#
#
#libcd_Toons_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-theme.c
#	applet-theme.h
#	applet-draw.c
#	applet-draw.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_Toons_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(toonsdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""Toons.conf""
#	-DMY_APPLET_USER_DATA_DIR=""Toons""
#	-DMY_APPLET_VERSION=""$(VERSION_TOONS)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_TOONS)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_Toons_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_Toons_la_LDFLAGS =
#	-avoid-version -module
