#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-gnome-integration],[1.0.2],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


gnome_integrationdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/gnome-integration]
AC_SUBST(gnome_integrationdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)

AC_SUBST(VERSION_GNOME_INTEGRATION, "$VERSION")
AC_SUBST(PACKAGE_GNOME_INTEGRATION, "$PACKAGE")

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(GNOME_INTEGRATION,gio-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile])
AC_OUTPUT
