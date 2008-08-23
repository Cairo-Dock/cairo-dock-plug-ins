 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-AlsaMixer.la


libcd_AlsaMixer_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-mixer.c\
	applet-mixer.h\
	applet-draw.h\
	applet-draw.c\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_AlsaMixer_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(ALSA_MIXER_PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(alsa_mixerdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""AlsaMixer.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""AlsaMixer"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_ALSA_MIXER)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_ALSA_MIXER)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_AlsaMixer_la_LIBADD = \
	$(PACKAGE_LIBS)\
	$(ALSA_MIXER_PACKAGE_LIBS)\
	 -lm


libcd_AlsaMixer_la_LDFLAGS =\
	-avoid-version -module
