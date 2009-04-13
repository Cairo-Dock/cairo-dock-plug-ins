#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-clock],[2.0.2],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CLOCK="cd-clock"
AC_DEFINE_UNQUOTED(GETTEXT_CLOCK,"$GETTEXT_CLOCK", [Gettext clock package.])
AC_SUBST(GETTEXT_CLOCK)

AC_SUBST(VERSION_CLOCK, "$VERSION")
AC_SUBST(PACKAGE_CLOCK, "$PACKAGE")

clockuserdirname="clock"
AC_SUBST(clockuserdirname)
clockdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/clock]
AC_SUBST(clockdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/clock.conf
		data/digital/Makefile
		data/digital/default/Makefile
		data/themes/Makefile
		data/themes/antique/Makefile
		data/themes/default/Makefile
		data/themes/fdo/Makefile
		data/themes/funky/Makefile
		data/themes/glassy/Makefile
		data/themes/ipulse/Makefile
		data/themes/radium/Makefile
		data/themes/silvia/Makefile
		data/themes/simple/Makefile
		data/themes/tango/Makefile
		data/themes/I_Clock/Makefile
		data/themes/BlueSlim/Makefile
		data/themes/AmoraSlim/Makefile
		data/themes/elegance/Makefile
		data/themes/zen/Makefile])
AC_OUTPUT
