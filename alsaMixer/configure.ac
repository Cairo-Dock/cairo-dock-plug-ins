#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-AlsaMixer],[1.0.10],[Fabrice Rey (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_ALSA_MIXER, "$VERSION")
AC_SUBST(PACKAGE_ALSA_MIXER, "$PACKAGE")

alsa_mixerdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/AlsaMixer]
AC_SUBST(alsa_mixerdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(ALSA_MIXER_PACKAGE, alsa)


AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/AlsaMixer.conf])
AC_OUTPUT
