#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-xfce-integration],[1.0.0],[Tofe (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


xfce_integrationdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/xfce-integration]
AC_SUBST(xfce_integrationdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_SUBST(VERSION_XFCE_INTEGRATION, "$VERSION")
AC_SUBST(PACKAGE_XFCE_INTEGRATION, "$PACKAGE")


PKG_CHECK_MODULES(PACKAGE, cairo-dock thunar-vfs-1)
PKG_CHECK_MODULES(XFCE_INTEGRATION,thunar-vfs-1)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/readme])
AC_OUTPUT
