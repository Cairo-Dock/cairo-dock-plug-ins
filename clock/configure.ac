#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-clock],[1.3.3],[Fabrice Rey (fabounet_03@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-clock
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/clock]
AC_SUBST(pkgdatadir)


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
		data/themes/zen/Makefile])
AC_OUTPUT
