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


libfile_manager_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DFILE_MANAGER_SHARE_DATA_DIR=\""$(datadir)/file-manager"\"\
	-DFILE_MANAGER_BACKEND_DIR=\""${libdir}"\"\
	-DFILE_MANAGER_README_FILE=\""readme"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libfile_manager_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libfile_manager_la_LDFLAGS =\
	-version-info 1:0:0


modulefile_managerdir = `pkg-config cairo-dock --variable=pluginsdir`

modulefile_manager_DATA = \
	.libs/libfile-manager.so


file_managerincludedir =\
	$(includedir)/file-manager

file_managerinclude_HEADERS =\
	file-manager-struct.h\
	file-manager-load-directory.h\
	file-manager-add-desktop-file.h
