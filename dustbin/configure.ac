#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dustbin],[1.2.1],[Fabrice Rey (fabounet_03@yahoo.fr)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL

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
		data/Makefile
		data/themes/Makefile
		data/themes/Gion/Makefile
		data/themes/Crystal/Makefile
		data/themes/NuoveXT-2/Makefile
		data/themes/OpenClipArt1/Makefile
		])
AC_OUTPUT
