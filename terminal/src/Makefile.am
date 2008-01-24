 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-terminal.la


libcd_terminal_la_SOURCES = \
	terminal-init.c\
	terminal-init.h\
	terminal-config.c\
	terminal-config.h\
	terminal-menu-functions.c\
	terminal-menu-functions.h\
	cairo-applet.c\
	cairo-applet.h\
	terminal-callbacks.c\
	terminal-callbacks.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_terminal_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""terminal.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""terminal"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
        -DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-std=c99\
	-O3 \
	$(LIBVTE_CFLAGS)
#	-g -ggdb -W -Wall


libcd_terminal_la_LIBADD = \
	$(PACKAGE_LIBS) -lm \
	$(LIBVTE_LIBS)



libcd_terminal_la_LDFLAGS =\
	-version-info 1:0:0


moduleterminaldir = `pkg-config cairo-dock --variable=pluginsdir`

moduleterminal_DATA = \
	.libs/libcd-terminal.so

