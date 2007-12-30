#dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libfile-manager.la


libfile_manager_la_SOURCES = \
	file-manager-init.c\
	file-manager-init.h\
	file-manager-config.c\
	file-manager-config.h\
	file-manager-menu-functions.c\
	file-manager-menu-functions.h\
	file-manager-add-desktop-file.c\
	file-manager-add-desktop-file.h\
	file-manager-load-directory.c\
	file-manager-load-directory.h


dock_version = `pkg-config --modversion cairo-dock`

libfile_manager_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_BACKEND_DIR=\""${libdir}"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""file-manager"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libfile_manager_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libfile_manager_la_LDFLAGS =\
	-version-info 1:0:0


moduledir = `pkg-config cairo-dock --variable=pluginsdir`

module_DATA = \
	.libs/libfile-manager.so


file_managerincludedir =\
	$(includedir)/file-manager

file_managerinclude_HEADERS =\
	file-manager-struct.h\
	file-manager-load-directory.h\
	file-manager-add-desktop-file.h
