#dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libfile-manager-gnome.la


libfile_manager_gnome_la_SOURCES = \
	file-manager-vfs-gnome.c\
	file-manager-vfs-gnome.h\
	file-manager-vfs-gnome-volumes.c\
	file-manager-vfs-gnome-volumes.h


libfile_manager_gnome_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libfile_manager_gnome_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libfile_manager_gnome_la_LDFLAGS =\
	-version-info 1:0:0

