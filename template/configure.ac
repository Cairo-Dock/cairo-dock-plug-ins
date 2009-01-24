#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-CD_APPLET_NAME],[0.0.1],[CD_MY_NAME (CD_MY_MAIL)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CD_PKG=cd-CD_APPLET_NAME
AC_DEFINE_UNQUOTED(GETTEXT_CD_PKG,"$GETTEXT_CD_PKG", [Gettext package.])
AC_SUBST(GETTEXT_CD_PKG)

AC_SUBST(VERSION_CD_PKG, "$VERSION")
AC_SUBST(PACKAGE_CD_PKG, "$PACKAGE")

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/CD_APPLET_NAME]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/CD_APPLET_NAME.conf])
AC_OUTPUT
