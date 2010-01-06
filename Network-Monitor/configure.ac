#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Network-Monitor],[0.2.0],[Yann SLADEK (mav@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_NETWORK_MONITOR, "$VERSION")
AC_SUBST(PACKAGE_NETWORK_MONITOR, "$PACKAGE")

network_monitoruserdirname="Network-Monitor"
AC_SUBST(network_monitoruserdirname)
network_monitordatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Network-Monitor]
AC_SUBST(network_monitordatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Network-Monitor.conf])
AC_OUTPUT
