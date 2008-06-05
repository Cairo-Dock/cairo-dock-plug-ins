#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock-plugins],[1.5.6],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=cairo-dock-plugins
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_SUBST (srcdir, ".")
AC_SUBST (top_srcdir, "..")
AC_SUBST (top_builddir, "..")


############# DUSTBIN #################
GETTEXT_DUSTBIN=cd-dustbin
AC_DEFINE_UNQUOTED(GETTEXT_DUSTBIN,"$GETTEXT_DUSTBIN", [Gettext dustbin package.])
AC_SUBST(GETTEXT_DUSTBIN)

AC_SUBST(VERSION_DUSTBIN, "2.1.2")
AC_SUBST(PACKAGE_DUSTBIN, "cd-dustbin")

AC_SUBST(dustbindatadir, "${pkgdatadir}/dustbin")

############# CLOCK #################
GETTEXT_CLOCK=cd-clock
AC_DEFINE_UNQUOTED(GETTEXT_CLOCK,"$GETTEXT_CLOCK", [Gettext clock package.])
AC_SUBST(GETTEXT_CLOCK)

AC_SUBST(VERSION_CLOCK, "1.3.4")
AC_SUBST(PACKAGE_CLOCK, "cd-clock")

AC_SUBST(clockdatadir, "${pkgdatadir}/clock")

############# LOGOUT #################
GETTEXT_LOGOUT=cd-logout
AC_DEFINE_UNQUOTED(GETTEXT_LOGOUT,"$GETTEXT_LOGOUT", [Gettext logout package.])
AC_SUBST(GETTEXT_LOGOUT)

AC_SUBST(VERSION_LOGOUT, "1.2.2")
AC_SUBST(PACKAGE_LOGOUT, "cd-logout")

AC_SUBST(logoutdatadir, "${pkgdatadir}/logout")


PKG_CHECK_MODULES(PACKAGE, cairo-dock gthread-2.0)

AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	
	dustbin/Makefile
	dustbin/src/Makefile
	dustbin/po/Makefile.in
	dustbin/data/Makefile
	dustbin/data/dustbin.conf
	dustbin/data/readme
	dustbin/data/themes/Makefile
	dustbin/data/themes/Gion/Makefile
	dustbin/data/themes/Crystal/Makefile
	dustbin/data/themes/NuoveXT-2/Makefile
	dustbin/data/themes/OpenClipArt1/Makefile
	dustbin/data/themes/DroplineNeu/Makefile
	dustbin/data/themes/Human/Makefile
	dustbin/data/themes/Metal/Makefile
	dustbin/data/themes/Silver/Makefile
	dustbin/data/themes/I_Dustbin/Makefile
	dustbin/data/themes/Neon/Makefile
	
	clock/Makefile
	clock/src/Makefile
	clock/po/Makefile.in
	clock/data/Makefile
	clock/data/readme
	clock/data/clock.conf
	clock/data/themes/Makefile
	clock/data/themes/antique/Makefile
	clock/data/themes/default/Makefile
	clock/data/themes/default-24/Makefile
	clock/data/themes/fdo/Makefile
	clock/data/themes/funky/Makefile
	clock/data/themes/glassy/Makefile
	clock/data/themes/ipulse/Makefile
	clock/data/themes/ipulse-24/Makefile
	clock/data/themes/quartz-24/Makefile
	clock/data/themes/radium/Makefile
	clock/data/themes/radium-24/Makefile
	clock/data/themes/silvia/Makefile
	clock/data/themes/silvia-24/Makefile
	clock/data/themes/simple/Makefile
	clock/data/themes/simple-24/Makefile
	clock/data/themes/tango/Makefile
	clock/data/themes/I_Clock/Makefile
	clock/data/themes/BlueSlim/Makefile
	clock/data/themes/zen/Makefile
	
	logout/Makefile
	logout/src/Makefile
	logout/po/Makefile.in
	logout/data/Makefile
	logout/data/readme
	logout/data/logout.conf
	])

AC_OUTPUT([po/Makefile
	dustbin/po/Makefile
	clock/po/Makefile
	logout/po/Makefile
	])
