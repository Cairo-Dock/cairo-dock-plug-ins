
########### sources ###############

SET(cd-switcher_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-load-icons.c
	applet-load-icons.h
	applet-draw.c
	applet-draw.h
	applet-desktops.c
	applet-desktops.h
	applet-struct.h
)

add_library(${PACKAGE_SWITCHER} SHARED ${cd-switcher_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${switcherdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="switcher.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="switcher")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SWITCHER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SWITCHER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="workspaces.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SWITCHER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SWITCHER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-switcher.la
#
#
#libcd_switcher_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-load-icons.c
#	applet-load-icons.h
#	applet-draw.c
#	applet-draw.h
#	applet-desktops.c
#	applet-desktops.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_switcher_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${switcherdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""switcher.conf""
#	-DMY_APPLET_USER_DATA_DIR=""switcher""
#	-DMY_APPLET_VERSION=""${VERSION_SWITCHER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_SWITCHER)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""workspaces.svg""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_switcher_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_switcher_la_LDFLAGS =
#	-avoid-version -module
