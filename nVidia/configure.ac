#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-nVidia],[1.0.7],[ChAnGFu (Robertson Rémy) (changfu@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_NVIDIA=cd-nVidia
AC_DEFINE_UNQUOTED(GETTEXT_NVIDIA,"$GETTEXT_NVIDIA", [Gettext package.])
AC_SUBST(GETTEXT_NVIDIA)

AC_SUBST(VERSION_NVIDIA, "$VERSION")
AC_SUBST(PACKAGE_NVIDIA, "$PACKAGE")

nvidiadatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/nVidia]
AC_SUBST(nvidiadatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/nVidia.conf])
AC_OUTPUT
