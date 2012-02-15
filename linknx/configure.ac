AC_PREREQ(2.60)
AC_INIT(linknx, 0.0.1.30, linknx@ouaye.net)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/linknx.cpp])
AC_CONFIG_HEADER([config.h])

PKG_CONFIG=`which pkg-config` 

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
		if test x"$enable_smtp" = xstatic ; then
			enable_smtp=static
		else
			enable_smtp=yes
		fi,
		if test x"$enable_smtp" = xyes -o x"$enable_smtp" = xstatic ; then
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
	if test x"$enable_smtp" = xstatic ; then
		
		LIBS=`echo $ESMTP_LIBS $LIBS | sed 's/-lesmtp/-Xlinker -Bstatic -lesmtp -Xlinker -Bdynamic/'`
	else
		LIBS="$ESMTP_LIBS $LIBS"
	fi
	AC_DEFINE([HAVE_LIBESMTP], [1], [Build with libesmtp email support.])
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi

dnl #########################################################################
dnl Check if using log4cpp
dnl #########################################################################

AC_PATH_PROG(LOG4CPP_CONFIG,log4cpp-config)
AC_ARG_WITH(log4cpp, AC_HELP_STRING([--with-log4cpp],[Advanced logging with Log4cpp]),
[WITH_LOG4CPP=$withval],[WITH_LOG4CPP=auto])

if test x"$WITH_LOG4CPP" != xno; then
  # try pkgconfig
  if test x"$WITH_LOG4CPP" = xyes ; then
    LOG4CPPPC=log4cpp
  elif test x"$WITH_LOG4CPP" = xauto ; then
    LOG4CPPPC=log4cpp
  else
    LOG4CPPPC=$WITH_LOG4CPP
  fi

  if test x"$WITH_LOG4CPP" = xauto ; then
    PKG_CHECK_MODULES(LOG4CPP, $LOG4CPPPC >= 1.0, [
      AC_DEFINE([HAVE_LOG4CPP], [1], [liblog4cpp])
      AC_SUBST(LOG4CPP_CFLAGS)
      AC_SUBST(LOG4CPP_LIBS)
    ],[
      AC_MSG_RESULT([no])    
    ])
  else
    PKG_CHECK_MODULES(LOG4CPP, $LOG4CPPPC >= 1.0, [
      AC_DEFINE([HAVE_LOG4CPP], [1], [liblog4cpp])
      AC_SUBST(LOG4CPP_CFLAGS)
      AC_SUBST(LOG4CPP_LIBS)
    ])
  fi
fi

dnl #########################################################################
dnl Check if using lua
dnl #########################################################################

AC_ARG_WITH(lua, AC_HELP_STRING([--with-lua],[Enable lua scripting engine]),
[WITH_LUA=$withval],[WITH_LUA=auto])

if test x"$WITH_LUA" != xno ; then
  # try pkgconfig
  if test x"$WITH_LUA" = xyes ; then
    LUAPC=lua
  elif test x"$WITH_LUA" = xauto ; then
    LUAPC=lua
  else
    LUAPC=$WITH_LUA
  fi

  if test x"$WITH_LUA" = xauto; then
    PKG_CHECK_MODULES(LUA, lua >= 5.1, [
      AC_DEFINE([HAVE_LUA], [1], [liblua])
      AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
      AC_SUBST(LUA_CFLAGS)
      AC_SUBST(LUA_LIBS)
    ],[
      AC_MSG_RESULT([no])    
      PKG_CHECK_MODULES(LUA, lua5.1 >= 5.1, [
        AC_DEFINE([HAVE_LUA], [1], [liblua])
        AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
        AC_SUBST(LUA_CFLAGS)
        AC_SUBST(LUA_LIBS)
      ],[
        AC_MSG_RESULT([no])    
      ])
    ])
  else
    PKG_CHECK_MODULES(LUA, $LUAPC >= 5.1, [
      AC_DEFINE([HAVE_LUA], [1], [liblua])
      AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
      AC_SUBST(LUA_CFLAGS)
      AC_SUBST(LUA_LIBS)
    ],[
      AC_MSG_RESULT([no])    
      PKG_CHECK_MODULES(LUA, "$LUAPC"5.1 >= 5.1, [
        AC_DEFINE([HAVE_LUA], [1], [liblua])
        AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
        AC_SUBST(LUA_CFLAGS)
        AC_SUBST(LUA_LIBS)
      ])
    ])
  fi
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT64_T
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

AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE([#include <sstream>
#include <stdint.h>],
[int64_t value; std::istringstream val("1234"); val >> value;],
AC_DEFINE(STL_STREAM_SUPPORT_INT64, [1], [STL streams has support for 64 bit integer]),)
AC_LANG_RESTORE


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 ticpp/Makefile
                 include/Makefile
                 test/Makefile
                 conf/Makefile])
AC_OUTPUT
