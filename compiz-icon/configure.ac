#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-compiz-icon],[1.1.2],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_COMPIZ_ICON=cd-compiz-icon
AC_DEFINE_UNQUOTED(GETTEXT_COMPIZ_ICON,"$GETTEXT_COMPIZ_ICON", [Gettext package.])
AC_SUBST(GETTEXT_COMPIZ_ICON)

AC_SUBST(VERSION_COMPIZ_ICON, "$VERSION")
AC_SUBST(PACKAGE_COMPIZ_ICON, "$PACKAGE")

compiz_icondatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/compiz-icon]
AC_SUBST(compiz_icondatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/compiz-icon.conf])
AC_OUTPUT
