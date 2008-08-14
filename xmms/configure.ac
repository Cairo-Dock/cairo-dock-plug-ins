#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-xmms],[0.2.3],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_XMMS=cd-xmms
AC_DEFINE_UNQUOTED(GETTEXT_XMMS,"$GETTEXT_XMMS", [Gettext package.])
AC_SUBST(GETTEXT_XMMS)

AC_SUBST(VERSION_XMMS, "$VERSION")
AC_SUBST(PACKAGE_XMMS, "$PACKAGE")

xmmsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/xmms]
AC_SUBST(xmmsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/xmms.conf])
AC_OUTPUT
