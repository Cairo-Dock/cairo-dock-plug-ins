 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-powermanager.la


libcd_powermanager_la_SOURCES = \
	powermanager-struct.h\
	powermanager-init.c\
	powermanager-init.h\
	powermanager-draw.c\
	powermanager-draw.h\
	powermanager-config.c\
	powermanager-config.h\
	powermanager-dbus.c\
	powermanager-dbus.h\
	powermanager-menu-functions.c\
	powermanager-menu-functions.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_powermanager_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""powermanager.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""powermanager"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall

libcd_powermanager_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_powermanager_la_LDFLAGS =\
	-avoid-version
