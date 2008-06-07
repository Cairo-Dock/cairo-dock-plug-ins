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
	systray-struct.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_systray_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(systraydatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""systray.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""systray"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_SYSTRAY)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_SYSTRAY)"\"\
        -DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99 -W -Werror\
	-O3
#	-g -ggdb -W -Wall

libcd_systray_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_systray_la_LDFLAGS =\
	-avoid-version
