#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-rhythmbox],[1.6.3],[Adrien Pilleboue (adrien.pilleboue@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_RHYTHMBOX=cd-rhythmbox
AC_DEFINE_UNQUOTED(GETTEXT_RHYTHMBOX,"$GETTEXT_RHYTHMBOX", [Gettext package.])
AC_SUBST(GETTEXT_RHYTHMBOX)

AC_SUBST(VERSION_RHYTHMBOX, "$VERSION")
AC_SUBST(PACKAGE_RHYTHMBOX, "$PACKAGE")

rhythmboxdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/rhythmbox]
AC_SUBST(rhythmboxdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, "cairo-dock dbus-glib-1 dbus-1")

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/rhythmbox.conf
		data/readme
		data/themes/Makefile
		data/themes/None/Makefile
		data/themes/cd_box_3d/Makefile
		data/themes/cd_box_simple/Makefile
		data/themes/super_jewel/Makefile])
AC_OUTPUT
