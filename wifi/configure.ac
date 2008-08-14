#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-wifi],[1.0.5],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_WIFI=cd-wifi
AC_DEFINE_UNQUOTED(GETTEXT_WIFI,"$GETTEXT_WIFI", [Gettext package.])
AC_SUBST(GETTEXT_WIFI)

AC_SUBST(VERSION_WIFI, "$VERSION")
AC_SUBST(PACKAGE_WIFI, "$PACKAGE")

wifidatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/wifi]
AC_SUBST(wifidatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/wifi.conf])
AC_OUTPUT
