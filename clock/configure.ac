#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-clock],[1.2.1],[Fabrice Rey (fabounet_03@yahoo.fr)])
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

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([sscanf])

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
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
		data/themes/zen/Makefile])
AC_OUTPUT
