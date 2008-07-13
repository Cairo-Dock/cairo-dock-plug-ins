 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-shortcuts.la


libcd_shortcuts_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-bookmarks.c\
	applet-bookmarks.h\
	applet-load-icons.c\
	applet-load-icons.h\
	applet-struct.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_shortcuts_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(shortcutsdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""shortcuts.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""shortcuts"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_SHORTCUTS)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_SHORTCUTS)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_shortcuts_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_shortcuts_la_LDFLAGS =\
	-avoid-version
