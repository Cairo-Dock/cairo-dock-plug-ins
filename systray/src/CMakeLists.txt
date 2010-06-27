
########### sources ###############

SET(cd-systray_LIB_SRCS
	systray-init.c
	systray-init.h
	systray-config.c
	systray-config.h
	systray-notifications.c
	systray-notifications.h
	systray-menu-functions.c
	systray-menu-functions.h
	na-tray.c
	na-tray.h
	na-tray-manager.c
	na-tray-manager.h
	na-tray-child.c
	na-tray-child.h
	na-marshal.h
	na-marshal.c
	fixedtip.c
	fixedtip.h
	systray-struct.h
)

add_library(${PACKAGE_SYSTRAY} SHARED ${cd-systray_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${systraydatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="systray.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="systray")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SYSTRAY}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SYSTRAY}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SYSTRAY}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SYSTRAY} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-systray.la
#
#
#libcd_systray_la_SOURCES = 
#	systray-init.c
#	systray-init.h
#	systray-config.c
#	systray-config.h
#	systray-menu-functions.c
#	systray-menu-functions.h
#	cd-tray.c
#	cd-tray.h
#	na-tray-manager.h
#	na-tray-manager.c
#	na-marshal.h
#	na-marshal.c
#	systray-struct.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_systray_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${systraydatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""systray.conf""
#	-DMY_APPLET_USER_DATA_DIR=""systray""
#	-DMY_APPLET_VERSION=""${VERSION_SYSTRAY)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_SYSTRAY)""
#        -DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall -W -Werror
#
#libcd_systray_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_systray_la_LDFLAGS =
#	-avoid-version -module
