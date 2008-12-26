 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-Animated-icons.la


libcd_Animated_icons_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-icon-renderer.c\
	applet-icon-renderer.h\
	applet-struct.h\
	applet-mesh-factory.c\
	applet-mesh-factory.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-spot.c\
	applet-spot.h\
	applet-rays.c\
	applet-rays.h\
	applet-wobbly.c\
	applet-wobbly.h\
	applet-wave.c\
	applet-wave.h\
	applet-pulse.c\
	applet-pulse.h\
	applet-bounce.c\
	applet-bounce.h\
	applet-blink.c\
	applet-blink.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_Animated_icons_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(animated_iconsdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""Animated-icons.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""Animated-icons"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_ANIMATED_ICONS)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_ANIMATED_ICONS)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_Animated_icons_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_Animated_icons_la_LDFLAGS =\
	-avoid-version -module
