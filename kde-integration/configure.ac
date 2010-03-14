#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd_kde-integration],[0.0.1],[Fabrice Rey (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


kde_integrationdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/kde-integration]
AC_SUBST(kde_integrationdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_SUBST(VERSION_KDE_INTEGRATION, "$VERSION")
AC_SUBST(PACKAGE_KDE_INTEGRATION, "$PACKAGE")

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(KDE_INTEGRATION,gio-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile])
AC_OUTPUT
