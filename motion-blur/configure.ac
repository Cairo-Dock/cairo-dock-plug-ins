#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-motion_blur],[1.0.0],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_MOTION_BLUR=cd-motion_blur
AC_DEFINE_UNQUOTED(GETTEXT_MOTION_BLUR,"$GETTEXT_MOTION_BLUR", [Gettext package.])
AC_SUBST(GETTEXT_MOTION_BLUR)

AC_SUBST(VERSION_MOTION_BLUR, "$VERSION")
AC_SUBST(PACKAGE_MOTION_BLUR, "$PACKAGE")


motion_blurdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/motion_blur]
AC_SUBST(motion_blurdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/motion_blur.conf])
AC_OUTPUT
