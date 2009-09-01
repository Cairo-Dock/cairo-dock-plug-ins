#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-netspeed],[1.2.0],[parAdOxxx_ZeRo (changfu@hollowproject.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_NETSPEED=cd-netspeed
AC_DEFINE_UNQUOTED(GETTEXT_NETSPEED,"$GETTEXT_NETSPEED", [Gettext package.])
AC_SUBST(GETTEXT_NETSPEED)

AC_SUBST(VERSION_NETSPEED, "$VERSION")
AC_SUBST(PACKAGE_NETSPEED, "$PACKAGE")

netspeeddatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/netspeed]
AC_SUBST(netspeeddatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/netspeed.conf])
AC_OUTPUT
