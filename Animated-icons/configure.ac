#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Animated-icons],[1.0.4],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_ANIMATED_ICONS, "$VERSION")
AC_SUBST(PACKAGE_ANIMATED_ICONS, "$PACKAGE")


animated_iconsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Animated-icons]
AC_SUBST(animated_iconsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Animated-icons.conf])
AC_OUTPUT
