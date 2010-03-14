#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-keyboard-indicator],[1.0.2],[Fabounet (Fabrice Rey) (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_KEYBOARD_INDICATOR, "$VERSION")
AC_SUBST(PACKAGE_KEYBOARD_INDICATOR, "$PACKAGE")

keyboard_indicatordatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/keyboard-indicator]
AC_SUBST(keyboard_indicatordatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(KEYBOARD_INDICATOR_PACKAGE, libxklavier)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/keyboard-indicator.conf])
AC_OUTPUT
