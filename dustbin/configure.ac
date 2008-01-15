#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dustbin],[2.0.0-beta2],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/dustbin]
AC_SUBST(pkgdatadir)


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-dustbin
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)


# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpopt':

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, sructures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([sscanf])

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/dustbin.conf
		data/readme
		data/themes/Makefile
		data/themes/Gion/Makefile
		data/themes/Crystal/Makefile
		data/themes/NuoveXT-2/Makefile
		data/themes/OpenClipArt1/Makefile
		data/themes/DroplineNeu/Makefile
		data/themes/Human/Makefile
		data/themes/Metal/Makefile
		data/themes/Silver/Makefile
		])
AC_OUTPUT
