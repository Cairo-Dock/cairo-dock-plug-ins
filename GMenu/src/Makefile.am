 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-GMenu.la


if WITH_GIO_IN_GMENU
have_gio=-DHAVE_GIO="1"
endif

libcd_GMenu_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-menu-callbacks.c\
	applet-menu-callbacks.h\
	applet-menu.c\
	applet-menu.h\
	applet-util.c\
	applet-util.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_GMenu_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(GMENU_PACKAGE_CFLAGS)\
	$(GMENU_GIO_CFLAGS) \
	$(have_gio)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(gmenudatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""GMenu.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""GMenu"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_GMENU)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_GMENU)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_GMenu_la_LIBADD = \
	$(PACKAGE_LIBS) -lm\
	$(GMENU_GIO_LIBS)\
	$(GMENU_PACKAGE_LIBS)


libcd_GMenu_la_LDFLAGS =\
	-avoid-version -module
