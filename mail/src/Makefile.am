 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-mail.la


libcd_mail_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-struct.h\
	mailwatch.c\
	mailwatch.h\
	mailwatch-mailbox-gmail.c\
	mailwatch-mailbox-imap.c\
	mailwatch-mailbox-pop3.c\
	mailwatch-mailbox-maildir.c\
	mailwatch-mailbox-mbox.c\
	mailwatch-mailbox-mh.c\
	mailwatch-mailbox.h\
	mailwatch-utils.c\
	mailwatch-utils.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_mail_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""mail.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""mail"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""cd_mail_icon.png"\"\
	-std=c99\
	-fgnu89-inline\
	-Werror-implicit-function-declaration\
	-g -ggdb -W
#	-O3


libcd_mail_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_mail_la_LDFLAGS =\
	-avoid-version
