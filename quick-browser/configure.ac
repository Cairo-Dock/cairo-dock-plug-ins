#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-quick-browser],[1.0.2],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_QUICK_BROWSER=cd-quick-browser
AC_DEFINE_UNQUOTED(GETTEXT_QUICK_BROWSER,"$GETTEXT_QUICK_BROWSER", [Gettext package.])
AC_SUBST(GETTEXT_QUICK_BROWSER)

AC_SUBST(VERSION_QUICK_BROWSER, "$VERSION")
AC_SUBST(PACKAGE_QUICK_BROWSER, "$PACKAGE")

quick_browserdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/quick-browser]
AC_SUBST(quick_browserdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/quick-browser.conf])
AC_OUTPUT
