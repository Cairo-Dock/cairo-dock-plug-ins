#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-weather],[1.2.2],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_WEATHER=cd-weather
AC_DEFINE_UNQUOTED(GETTEXT_WEATHER,"$GETTEXT_WEATHER", [Gettext package.])
AC_SUBST(GETTEXT_WEATHER)

AC_SUBST(VERSION_WEATHER, "$VERSION")
AC_SUBST(PACKAGE_WEATHER, "$PACKAGE")

weatheruserdirname="weather"
AC_SUBST(weatheruserdirname)
weatherdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/weather]
AC_SUBST(weatherdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock libxml-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/weather.conf
		data/themes/Makefile
		data/themes/basic/Makefile
		data/themes/Shiny/Makefile
		data/themes/crystal/Makefile
		data/themes/7degrees/Makefile
		data/themes/clear/Makefile
		data/themes/I_Weather/Makefile
		data/themes/WeatherPod/Makefile])
AC_OUTPUT
