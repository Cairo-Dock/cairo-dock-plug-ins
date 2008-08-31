#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-stack],[0.2.1],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_STACK=cd-stack
AC_DEFINE_UNQUOTED(GETTEXT_STACK,"$GETTEXT_STACK", [Gettext package.])
AC_SUBST(GETTEXT_STACK)

AC_SUBST(VERSION_STACK, "$VERSION")
AC_SUBST(PACKAGE_STACK, "$PACKAGE")

stackdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/stack]
AC_SUBST(stackdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/stack.conf])
AC_OUTPUT
