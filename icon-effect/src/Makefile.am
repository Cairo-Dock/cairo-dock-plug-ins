 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-icon-effect.la


libcd_icon_effect_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-fire.c\
	applet-fire.h\
	applet-rain.c\
	applet-rain.h\
	applet-snow.c\
	applet-snow.h\
	applet-star.c\
	applet-star.h\
	applet-storm.c\
	applet-storm.h\
	fire-tex.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_icon_effect_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(icon_effectsdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""icon-effect.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""icon-effect"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_ICON_EFFECTS)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_ICON_EFFECTS)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
        -DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_icon_effect_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_icon_effect_la_LDFLAGS =\
	-avoid-version -module
