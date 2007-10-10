AC_PREREQ(2.60)
AC_INIT(linknx, 0.0.1.17, linknx@ouaye.net)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/linknx.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PTHSEM(2.0.4,yes,yes,no)
AC_CHECK_HEADER(argp.h,,[AC_MSG_ERROR([argp_parse not found])])
AC_SEARCH_LIBS(argp_parse,argp,,[AC_MSG_ERROR([argp_parse not found])])

# Checks for libraries.
LIBCURL_CHECK_CONFIG([yes], [7.14.0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h unistd.h])

dnl #########################################################################
dnl Check if using libesmtp
dnl #########################################################################

AC_ARG_ENABLE([smtp],
	AC_HELP_STRING([--enable-smtp], [build with libesmtp email support (default=auto)]),
	,
	enable_smtp=auto)
 
if test x"$enable_smtp" != xno ; then
	AC_CHECK_HEADER([libesmtp.h],
		enable_smtp=yes,
		if test x"$enable_smtp" = xyes ; then
			AC_MSG_ERROR([Cannot find libesmtp.])
		else
			enable_smtp=no
		fi
	)
fi

AC_MSG_CHECKING(whether to use libesmtp)
if test x"$enable_smtp" != xno ; then
	ESMTP_CFLAGS="`libesmtp-config --cflags`"
	ESMTP_LIBS="`libesmtp-config --libs`"
	CFLAGS="$ESMTP_CFLAGS $CFLAGS"
	LIBS="$ESMTP_LIBS $LIBS"
	AC_DEFINE([HAVE_LIBESMTP], [1], [Build with libesmtp email support.])
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 memmove memset strchr])

AC_DEFINE(TIXML_USE_TICPP, [1], [Use TICPP interface])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 ticpp/Makefile
                 include/Makefile
                 conf/Makefile])
AC_OUTPUT
