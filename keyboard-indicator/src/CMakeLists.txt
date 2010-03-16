
########### sources ###############

SET(cd-keyboard-indicator_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-xklavier.c
	applet-xklavier.h
	applet-draw.c
	applet-draw.h
	applet-struct.h
)

add_library(${PACKAGE_KEYBOARD_INDICATOR} SHARED ${cd-keyboard-indicator_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${keyboard_indicatordatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="keyboard-indicator.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="keyboard-indicator")
add_definitions (-DMY_APPLET_VERSION="${VERSION_KEYBOARD_INDICATOR}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_KEYBOARD_INDICATOR}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${KEYBOARD_INDICATOR_PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${KEYBOARD_INDICATOR_PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_KEYBOARD_INDICATOR}
	${PACKAGE_LIBRARIES}
	${KEYBOARD_INDICATOR_PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_KEYBOARD_INDICATOR} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-keyboard-indicator.la
#
#
#libcd_keyboard_indicator_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-xklavier.c
#	applet-xklavier.h
#	applet-draw.c
#	applet-draw.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_keyboard_indicator_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(KEYBOARD_INDICATOR_PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${keyboard_indicatordatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""keyboard-indicator.conf""
#	-DMY_APPLET_USER_DATA_DIR=""keyboard-indicator""
#	-DMY_APPLET_VERSION=""${VERSION_KEYBOARD_INDICATOR)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_KEYBOARD_INDICATOR)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_keyboard_indicator_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#	$(KEYBOARD_INDICATOR_PACKAGE_LIBS)
#
#
#libcd_keyboard_indicator_la_LDFLAGS =
#	-avoid-version -module
