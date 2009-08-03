#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Network-Monitor],[0.1],[Yann SLADEK (mav@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_NETWORK_MONITOR=cd-Network-Monitor
AC_DEFINE_UNQUOTED(GETTEXT_NETWORK_MONITOR,"$GETTEXT_NETWORK_MONITOR", [Gettext package.])
AC_SUBST(GETTEXT_NETWORK_MONITOR)

AC_SUBST(VERSION_NETWORK_MONITOR, "$VERSION")
AC_SUBST(PACKAGE_NETWORK_MONITOR, "$PACKAGE")

network_monitoruserdirname="Network-Monitor"
AC_SUBST(network_monitoruserdirname)
network_monitordatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Network-Monitor]
AC_SUBST(network_monitordatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/Network-Monitor.conf])
AC_OUTPUT
