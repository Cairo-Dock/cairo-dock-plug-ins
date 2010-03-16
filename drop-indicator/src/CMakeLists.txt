
########### sources ###############

SET(cd-drop_indicator_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
	bilinear-gradation-texture.h
)

add_library(${PACKAGE_DROP_INDICATOR} SHARED ${cd-drop_indicator_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(drop_indicatordatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="drop_indicator.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="drop_indicator")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_DROP_INDICATOR)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_DROP_INDICATOR)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DMY_APPLET_DEFAULT_DROP_INDICATOR_NAME="default-drop-indicator.svg")
add_definitions (-DMY_APPLET_DEFAULT_HOVER_INDICATOR_NAME="default-hover-indicator.svg")
add_definitions (-DMY_APPLET_MASK_INDICATOR_NAME="texture-bilinear-gradation.png")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DROP_INDICATOR}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_DROP_INDICATOR} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-drop_indicator.la
#
#
#libcd_drop_indicator_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#	bilinear-gradation-texture.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_drop_indicator_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(drop_indicatordatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""drop_indicator.conf""
#	-DMY_APPLET_USER_DATA_DIR=""drop_indicator""
#	-DMY_APPLET_VERSION=""$(VERSION_DROP_INDICATOR)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_DROP_INDICATOR)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-DMY_APPLET_DEFAULT_DROP_INDICATOR_NAME=""default-drop-indicator.svg""
#	-DMY_APPLET_DEFAULT_HOVER_INDICATOR_NAME=""default-hover-indicator.svg""
#	-DMY_APPLET_MASK_INDICATOR_NAME=""texture-bilinear-gradation.png""
#        -DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_drop_indicator_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_drop_indicator_la_LDFLAGS =
#	-avoid-version -module
