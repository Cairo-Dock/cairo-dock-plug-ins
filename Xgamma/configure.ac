#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Xgamma],[1.0.5],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_XGAMMA=cd-Xgamma
AC_DEFINE_UNQUOTED(GETTEXT_XGAMMA,"$GETTEXT_XGAMMA", [Gettext package.])
AC_SUBST(GETTEXT_XGAMMA)

AC_SUBST(VERSION_XGAMMA, "$VERSION")
AC_SUBST(PACKAGE_XGAMMA, "$PACKAGE")

xgammadatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Xgamma]
AC_SUBST(xgammadatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock x11 xxf86vm)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/Xgamma.conf])
AC_OUTPUT
