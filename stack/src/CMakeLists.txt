
########### sources ###############

SET(cd-stack_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-load-icons.c
	applet-load-icons.h
	applet-stack.c
	applet-stack.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_STACK} SHARED ${cd-stack_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(stackdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="stack.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="stack")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_STACK)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_STACK)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_STACK_DEFAULT_TEXT_ICON="text-icon.svg")
add_definitions (-DCD_STACK_DEFAULT_URL_ICON="url-icon.svg")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_STACK}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_STACK} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-stack.la
#
#
#libcd_stack_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-load-icons.c
#	applet-load-icons.h
#	applet-stack.c
#	applet-stack.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_stack_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(stackdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""stack.conf""
#	-DMY_APPLET_USER_DATA_DIR=""stack""
#	-DMY_APPLET_VERSION=""$(VERSION_STACK)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_STACK)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_STACK_DEFAULT_TEXT_ICON=""text-icon.svg""
#	-DCD_STACK_DEFAULT_URL_ICON=""url-icon.svg""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-Werror-implicit-function-declaration
#	-std=c99
##	-g -ggdb -W -Wall
#
#
#libcd_stack_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_stack_la_LDFLAGS =
#	-avoid-version -module
