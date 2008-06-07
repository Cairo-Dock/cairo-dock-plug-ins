#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-stacks],[0.1.0-RC],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_STACKS=cd-stacks
AC_DEFINE_UNQUOTED(GETTEXT_STACKS,"$GETTEXT_STACKS", [Gettext package.])
AC_SUBST(GETTEXT_STACKS)

AC_SUBST(VERSION_STACKS, "$VERSION")
AC_SUBST(PACKAGE_STACKS, "$PACKAGE")

stacksdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/stacks]
AC_SUBST(stacksdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/stacks.conf])
AC_OUTPUT
