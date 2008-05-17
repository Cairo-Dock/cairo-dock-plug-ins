#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-gnome-integration-old],[1.0.1],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/gnome-integration-old]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock gnome-vfs-2.0 libgnomeui-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/readme])
AC_OUTPUT
