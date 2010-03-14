#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd_xfce-integration],[1.0.0],[Tofe (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


xfce_integrationdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/xfce-integration]
AC_SUBST(xfce_integrationdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_SUBST(VERSION_XFCE_INTEGRATION, "$VERSION")
AC_SUBST(PACKAGE_XFCE_INTEGRATION, "$PACKAGE")

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(THUNAR_VFS,thunar-vfs-1)
AC_SUBST(THUNAR_VFS_CFLAGS)
AC_SUBST(THUNAR_VFS_LIBS)
PKG_CHECK_MODULES(LIBGIO,gio-2.0)
AC_SUBST(LIBGIO_CFLAGS)
AC_SUBST(LIBGIO_LIBS)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		])
AC_OUTPUT
