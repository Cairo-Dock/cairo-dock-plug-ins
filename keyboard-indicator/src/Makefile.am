 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-keyboard-indicator.la


libcd_keyboard_indicator_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-xklavier.c\
	applet-xklavier.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_keyboard_indicator_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(KEYBOARD_INDICATOR_PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""keyboard-indicator.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""keyboard-indicator"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_KEYBOARD_INDICATOR)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_keyboard_indicator_la_LIBADD = \
	$(PACKAGE_LIBS) -lm\
	$(KEYBOARD_INDICATOR_PACKAGE_LIBS)


libcd_keyboard_indicator_la_LDFLAGS =\
	-avoid-version -module
