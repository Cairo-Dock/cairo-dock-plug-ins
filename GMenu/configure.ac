#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-GMenu],[0.0.1],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_GMENU=cd-GMenu
AC_DEFINE_UNQUOTED(GETTEXT_GMENU,"$GETTEXT_GMENU", [Gettext package.])
AC_SUBST(GETTEXT_GMENU)

AC_SUBST(VERSION_GMENU, "$VERSION")
AC_SUBST(PACKAGE_GMENU, "$PACKAGE")

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(GMENU_PACKAGE, libgnome-menu gio-2.0)

gmenudatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/GMenu]
AC_SUBST(gmenudatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/GMenu.conf])
AC_OUTPUT
