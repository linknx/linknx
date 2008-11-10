AC_PREREQ(2.60)
AC_INIT(linknx, 0.0.1.26, linknx@ouaye.net)
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

dnl Check for CPPUnit 
ifdef([AM_PATH_CPPUNIT], [AM_PATH_CPPUNIT(1.9.6, [AM_CONDITIONAL(CPPUNIT, true)], [AM_CONDITIONAL(CPPUNIT, false)])],  
                         [AM_CONDITIONAL(CPPUNIT, false)]) 

# Checks for libraries.
LIBCURL_CHECK_CONFIG([yes], [7.14.0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h unistd.h])

AC_CHECK_MEMBER(struct tm.tm_gmtoff,
    [member_struct_tm_tm_gmtoff=yes],
    [member_struct_tm_tm_gmtoff=no],
    [#include <time.h>])
if test "$member_struct_tm_tm_gmtoff" = yes; then
    AC_DEFINE(HAVE_TM_GMTOFF,1,
        [Define if the tm_gmtoff member is present in struct tm.])
else
    AC_CHECK_MEMBER(struct tm.__tm_gmtoff,
        [member_struct_tm___tm_gmtoff=yes],
        [member_struct_tm___tm_gmtoff=no],
        [#include <time.h>])
    if test "$member_struct_tm___tm_gmtoff" = yes; then
        AC_DEFINE(HAVE___TM_GMTOFF,1,
            [Define if the __tm_gmtoff member is present in struct tm.])
    fi
fi

WITH_MYSQL()

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
	if test x"`libesmtp-config --libs | grep pthread`" != x ; then
		AC_MSG_WARN([libesmtp is compiled with pthread support. This can conflict with pth. If you observe segmentation faults at startup, try to recompile with libesmtp support disabled])
		AC_DEFINE([HAVE_LIBESMTP_PTHREAD], [1], [libesmtp is compiled with pthread support.])
	fi
	CFLAGS="$ESMTP_CFLAGS $CFLAGS"
	LIBS="$ESMTP_LIBS $LIBS"
	AC_DEFINE([HAVE_LIBESMTP], [1], [Build with libesmtp email support.])
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi

dnl #########################################################################
dnl Check if using lua
dnl #########################################################################

AC_ARG_ENABLE([lua],
	AC_HELP_STRING([--enable-lua], [build with lua scripting support (default=auto)]),
	,
	enable_lua=auto)
 
if test x"$enable_lua" != xno ; then
	AC_CHECK_HEADER([lua.h],
		enable_lua=yes,
		if test x"$enable_lua" = xyes ; then
			AC_MSG_ERROR([Cannot find lua header file.])
		else
			enable_lua=no
		fi
	)
fi

AC_MSG_CHECKING(whether to use lua)
if test x"$enable_lua" != xno ; then
	LUA_CFLAGS="`pkg-config lua  --cflags`"
	LUA_LIBS="`pkg-config lua --libs`"
	CFLAGS="$LUA_CFLAGS $CFLAGS"
	LIBS="$LUA_LIBS $LIBS"
	AC_DEFINE([HAVE_LUA], [1], [Build with lua scripting support.])
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
                 test/Makefile
                 conf/Makefile])
AC_OUTPUT
