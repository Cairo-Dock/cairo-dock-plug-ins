 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-musicplayer.la


libcd_musicplayer_la_SOURCES = \
	musicplayer-init.c\
	musicplayer-init.h\
	musicplayer-config.c\
	musicplayer-config.h\
	musicplayer-notifications.c\
	musicplayer-notifications.h\
	musicplayer-dbus.c\
	musicplayer-dbus.h\
	musicplayer-draw.c\
	musicplayer-draw.h\
	musicplayer-banshee.c\
	musicplayer-banshee.h\
	musicplayer-exaile.c\
	musicplayer-exaile.h\
	musicplayer-listen.c\
	musicplayer-listen.h\
	musicplayer-rhythmbox.c\
	musicplayer-rhythmbox.h\
	musicplayer-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_musicplayer_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""musicplayer.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""musicplayer"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_musicplayer_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_musicplayer_la_LDFLAGS =\
	-version-info 1:1:0


plugindir = `pkg-config cairo-dock --variable=pluginsdir`

plugin_DATA = \
	.libs/libcd-musicplayer.so

