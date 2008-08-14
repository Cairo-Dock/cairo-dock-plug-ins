#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-clock],[1.3.7],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_CLOCK="cd-clock"
AC_DEFINE_UNQUOTED(GETTEXT_CLOCK,"$GETTEXT_CLOCK", [Gettext clock package.])
AC_SUBST(GETTEXT_CLOCK)

AC_SUBST(VERSION_CLOCK, "$VERSION")
AC_SUBST(PACKAGE_CLOCK, "$PACKAGE")

clockdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/clock]
AC_SUBST(clockdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/clock.conf
		data/themes/Makefile
		data/themes/antique/Makefile
		data/themes/default/Makefile
		data/themes/default-24/Makefile
		data/themes/fdo/Makefile
		data/themes/funky/Makefile
		data/themes/glassy/Makefile
		data/themes/ipulse/Makefile
		data/themes/ipulse-24/Makefile
		data/themes/quartz-24/Makefile
		data/themes/radium/Makefile
		data/themes/radium-24/Makefile
		data/themes/silvia/Makefile
		data/themes/silvia-24/Makefile
		data/themes/simple/Makefile
		data/themes/simple-24/Makefile
		data/themes/tango/Makefile
		data/themes/I_Clock/Makefile
		data/themes/BlueSlim/Makefile
		data/themes/AmoraSlim/Makefile
		data/themes/zen/Makefile])
AC_OUTPUT
