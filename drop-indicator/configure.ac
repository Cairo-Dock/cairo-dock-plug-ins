#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-drop_indicator],[1.0.0],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-drop_indicator
AC_DEFINE_UNQUOTED(GETTEXT_DROP_INDICATOR,"$GETTEXT_DROP_INDICATOR", [Gettext package.])
AC_SUBST(GETTEXT_DROP_INDICATOR)

AC_SUBST(VERSION_DROP_INDICATOR, "$VERSION")
AC_SUBST(PACKAGE_DROP_INDICATOR, "$PACKAGE")


drop_indicatordatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/drop_indicator]
AC_SUBST(drop_indicatordatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/drop_indicator.conf])
AC_OUTPUT
