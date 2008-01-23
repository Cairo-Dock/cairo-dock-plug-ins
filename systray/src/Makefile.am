 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-systray.la


libcd_systray_la_SOURCES = \
	systray-init.c\
	systray-init.h\
	systray-config.c\
	systray-config.h\
	systray-menu-functions.c\
	systray-menu-functions.h\
	cd-tray.c\
	cd-tray.h\
	na-tray-manager.h\
	na-tray-manager.c\
	na-marshal.h\
	na-marshal.c\
	cairo-applet.c\
	cairo-applet.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_systray_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""systray.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""systray"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-systray"\"\
        -DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall

libcd_systray_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_systray_la_LDFLAGS =\
	-version-info 1:0:0


modulesystraydir = `pkg-config cairo-dock --variable=pluginsdir`

modulesystray_DATA = \
	.libs/libcd-systray.so

