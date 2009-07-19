#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Clipper],[1.0.3],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CLIPPER=cd-Clipper
AC_DEFINE_UNQUOTED(GETTEXT_CLIPPER,"$GETTEXT_CLIPPER", [Gettext package.])
AC_SUBST(GETTEXT_CLIPPER)

AC_SUBST(VERSION_CLIPPER, "$VERSION")
AC_SUBST(PACKAGE_CLIPPER, "$PACKAGE")

Clipperdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Clipper]
AC_SUBST(Clipperdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/Clipper.conf])
AC_OUTPUT
