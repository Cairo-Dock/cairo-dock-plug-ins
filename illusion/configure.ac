#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-illusion],[0.4.0],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_ILLUSION=cd-illusion
AC_DEFINE_UNQUOTED(GETTEXT_ILLUSION,"$GETTEXT_ILLUSION", [Gettext package.])
AC_SUBST(GETTEXT_ILLUSION)

AC_SUBST(VERSION_ILLUSION, "$VERSION")
AC_SUBST(PACKAGE_ILLUSION, "$PACKAGE")


illusiondatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/illusion]
AC_SUBST(illusiondatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/illusion.conf])
AC_OUTPUT
