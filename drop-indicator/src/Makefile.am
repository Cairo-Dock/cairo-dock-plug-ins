 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-drop_indicator.la


libcd_drop_indicator_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h\
	bilinear-gradation-texture.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_drop_indicator_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(drop_indicatordatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""drop_indicator.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""drop_indicator"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_DROP_INDICATOR)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_DROP_INDICATOR)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-DMY_APPLET_DEFAULT_DROP_INDICATOR_NAME=\""default-drop-indicator.svg"\"\
	-DMY_APPLET_DEFAULT_HOVER_INDICATOR_NAME=\""default-hover-indicator.svg"\"\
	-DMY_APPLET_MASK_INDICATOR_NAME=\""texture-bilinear-gradation.png"\"\
        -DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_drop_indicator_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_drop_indicator_la_LDFLAGS =\
	-avoid-version -module
