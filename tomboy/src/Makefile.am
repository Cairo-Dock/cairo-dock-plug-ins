 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-tomboy.la


libcd_tomboy_la_SOURCES = \
	tomboy-init.c\
	tomboy-init.h\
	tomboy-draw.c\
	tomboy-draw.h\
	tomboy-config.c\
	tomboy-config.h\
	tomboy-dbus.c\
	tomboy-dbus.h\
	tomboy-notifications.c\
	tomboy-notifications.h\
	tomboy-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_tomboy_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""tomboy.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""tomboy"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_tomboy_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_tomboy_la_LDFLAGS =\
	-avoid-version
