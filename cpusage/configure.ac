#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-cpusage],[1.2.0],[parAdOxxx_ZeRo (changfu@hollowproject.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CPUSAGE=cd-cpusage
AC_DEFINE_UNQUOTED(GETTEXT_CPUSAGE,"$GETTEXT_CPUSAGE", [Gettext package.])
AC_SUBST(GETTEXT_CPUSAGE)

AC_SUBST(VERSION_CPUSAGE, "$VERSION")
AC_SUBST(PACKAGE_CPUSAGE, "$PACKAGE")

cpusagedatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/cpusage]
AC_SUBST(cpusagedatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/cpusage.conf])
AC_OUTPUT
