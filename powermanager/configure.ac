#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-powermanager],[1.2.17],[Adrien Pilleboue (adrien.pilleboue@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_POWERMANAGER, "$VERSION")
AC_SUBST(PACKAGE_POWERMANAGER, "$PACKAGE")

powermanagerdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/powermanager]
AC_SUBST(powermanagerdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, "cairo-dock dbus-glib-1 dbus-1")

AC_CONFIG_FILES([Makefile
	src/Makefile
	data/Makefile
	data/powermanager.conf])
AC_OUTPUT
