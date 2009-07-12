 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-dnd2share.la


libcd_dnd2share_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-dnd2share.c\
	applet-dnd2share.h\
	applet-backend-uppix.c\
	applet-backend-uppix.h\
	applet-backend-imagebin.c\
	applet-backend-imagebin.h\
	applet-backend-imageshack.c\
	applet-backend-imageshack.h\
	applet-backend-free.c\
	applet-backend-free.h\
	applet-backend-pastebin.c\
	applet-backend-pastebin.h\
	applet-backend-custom.c\
	applet-backend-custom.h\
	applet-backend-dropbox.c\
	applet-backend-dropbox.h\
	applet-backend-videobin.c\
	applet-backend-videobin.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_dnd2share_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(dnd2sharedatadir)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""dnd2share.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""dnd2share"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_DND2SHARE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_dnd2share_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_dnd2share_la_LDFLAGS =\
	-avoid-version -module
