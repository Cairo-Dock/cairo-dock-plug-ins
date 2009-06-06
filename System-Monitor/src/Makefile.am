 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-System-Monitor.la


libcd_System_Monitor_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-cpusage.c\
	applet-cpusage.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_System_Monitor_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(system_monitordatadir)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""System-Monitor.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""System-Monitor"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_SYSTEM_MONITOR)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_SYSTEM_MONITOR)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_System_Monitor_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_System_Monitor_la_LDFLAGS =\
	-avoid-version -module
