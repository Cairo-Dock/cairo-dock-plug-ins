#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Dbus],[0.5.0],[Fabrice Rey (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_DBUS, "$VERSION")
AC_SUBST(PACKAGE_DBUS, "$PACKAGE")

dbusdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Dbus]
AC_SUBST(dbusdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Dbus.conf])
AC_OUTPUT
