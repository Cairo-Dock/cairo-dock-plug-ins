#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-keyboard-indicator],[1.0.2],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_KEYBOARD_INDICATOR=cd-keyboard-indicator
AC_DEFINE_UNQUOTED(GETTEXT_KEYBOARD_INDICATOR,"$GETTEXT_KEYBOARD_INDICATOR", [Gettext package.])
AC_SUBST(GETTEXT_KEYBOARD_INDICATOR)

AC_SUBST(VERSION_KEYBOARD_INDICATOR, "$VERSION")
AC_SUBST(PACKAGE_KEYBOARD_INDICATOR, "$PACKAGE")

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/keyboard-indicator]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(KEYBOARD_INDICATOR_PACKAGE, libxklavier)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/keyboard-indicator.conf])
AC_OUTPUT
