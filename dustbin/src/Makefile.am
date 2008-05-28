 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-dustbin.la


libcd_dustbin_la_SOURCES = \
	applet-struct.h\
	applet-init.c\
	applet-init.h\
	applet-draw.c\
	applet-draw.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-trashes-manager.c\
	applet-trashes-manager.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_dustbin_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""dustbin.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""dustbin"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_DUSTBIN)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_DUSTBIN)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_dustbin_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_dustbin_la_LDFLAGS =\
	-avoid-version
