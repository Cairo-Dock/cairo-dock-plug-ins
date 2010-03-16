
########### sources ###############

SET(cd-powermanager_LIB_SRCS
	powermanager-struct.h
	powermanager-init.c
	powermanager-init.h
	powermanager-draw.c
	powermanager-draw.h
	powermanager-config.c
	powermanager-config.h
	powermanager-dbus.c
	powermanager-dbus.h
	powermanager-menu-functions.c
	powermanager-menu-functions.h
)

add_library(${PACKAGE_POWERMANAGER} SHARED ${cd-powermanager_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${powermanagerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="powermanager.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="powermanager")
add_definitions (-DMY_APPLET_VERSION="${VERSION_POWERMANAGER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_POWERMANAGER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_POWERMANAGER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_POWERMANAGER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-powermanager.la
#
#
#libcd_powermanager_la_SOURCES = 
#	powermanager-struct.h
#	powermanager-init.c
#	powermanager-init.h
#	powermanager-draw.c
#	powermanager-draw.h
#	powermanager-config.c
#	powermanager-config.h
#	powermanager-dbus.c
#	powermanager-dbus.h
#	powermanager-menu-functions.c
#	powermanager-menu-functions.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_powermanager_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${powermanagerdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""powermanager.conf""
#	-DMY_APPLET_USER_DATA_DIR=""powermanager""
#	-DMY_APPLET_VERSION=""${VERSION_POWERMANAGER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_POWERMANAGER)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#libcd_powermanager_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_powermanager_la_LDFLAGS =
#	-avoid-version -module
