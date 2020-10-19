AC_PREREQ(2.60)
AC_INIT(linknx, 0.0.1.38, cyrille.github@youplaboum.fr)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/linknx.cpp])
AC_CONFIG_HEADER([config.h])

PKG_CONFIG=`which pkg-config`

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AM_ICONV
AC_CHECK_PTHSEM(2.0.4,yes,yes,no)
AC_CHECK_HEADER(argp.h,,[AC_MSG_ERROR([argp_parse not found])])
AC_SEARCH_LIBS(argp_parse,argp,,[AC_MSG_ERROR([argp_parse not found])])

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

dnl check for mysql
AC_ARG_WITH(mysql, AC_HELP_STRING([--with-mysql@<:@=DIR@>:@],
            [Include support for mysql]),
            mysql=$withval, mysql=yes)
if test "$mysql" != "no"; then
    if test "$mysql" != "yes"; then
        AC_CHECK_PROG([MYSQL_CONFIG],[mysql_config],[$mysql/bin/mysql_config],[no],[$mysql/bin])
    else
        AC_CHECK_PROG([MYSQL_CONFIG],[mysql_config],[mysql_config],[no])
    fi
    if test "$MYSQL_CONFIG" != "no"; then
        MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`
        MYSQL_LIBS=`$MYSQL_CONFIG --libs`
        AC_SUBST(MYSQL_CFLAGS)
        AC_SUBST(MYSQL_LIBS)
    else
        mysql=no
    fi
fi
AC_MSG_CHECKING([for mysql])
AC_MSG_RESULT($mysql)
if test "$mysql" != "no"; then
    AC_DEFINE(HAVE_MYSQL,,[mysql is available])
fi

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
AM_CONDITIONAL([USE_B64], [test x"$enable_smtp" != xno])

dnl #########################################################################
dnl Check if using cppunit
dnl #########################################################################

AC_ARG_WITH(cppunit, AC_HELP_STRING([--with-cppunit],[Build CppUnit testbench for unit tests]),
[WITH_CPPUNIT=$withval],[WITH_CPPUNIT=auto])

if test x"$WITH_CPPUNIT" != xno; then
  # try pkgconfig
  if test x"$WITH_CPPUNIT" = xyes ; then
    CPPUNITPC=cppunit
  elif test x"$WITH_CPPUNIT" = xauto ; then
    CPPUNITPC=cppunit
  else
    CPPUNITPC=$WITH_CPPUNIT
  fi

  if test x"$WITH_CPPUNIT" = xauto ; then
    PKG_CHECK_MODULES(CPPUNIT, $CPPUNITPC >= 1.9.6, [
      AC_DEFINE([HAVE_CPPUNIT], [1], [CppUnit is available])
      AC_SUBST(CPPUNIT_CFLAGS)
      AC_SUBST(CPPUNIT_LIBS)
      found_cppunit=yes
    ],[
      AC_MSG_RESULT([no])
    ])
  else
    PKG_CHECK_MODULES(CPPUNIT, $CPPUNITPC >= 1.9.6, [
      AC_DEFINE([HAVE_CPPUNIT], [1], [CppUnit is available])
      AC_SUBST(CPPUNIT_CFLAGS)
      AC_SUBST(CPPUNIT_LIBS)
      found_cppunit=yes
    ])
  fi
fi
AM_CONDITIONAL([CPPUNIT], [test x"$found_cppunit" = xyes])

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
      AC_DEFINE([HAVE_LOG4CPP], [1], [Log4cpp is available])
      AC_SUBST(LOG4CPP_CFLAGS)
      AC_SUBST(LOG4CPP_LIBS)
    ],[
      AC_MSG_RESULT([no])
    ])
  else
    PKG_CHECK_MODULES(LOG4CPP, $LOG4CPPPC >= 1.0, [
      AC_DEFINE([HAVE_LOG4CPP], [1], [Log4cpp is available])
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

  PKG_CHECK_MODULES([LUA], "$LUAPC" >= 5.1, [
    AC_DEFINE([HAVE_LUA], [1], [liblua])
     AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
     AC_SUBST(LUA_CFLAGS)
     AC_SUBST(LUA_LIBS)
  ],[
    PKG_CHECK_MODULES([LUA], "$LUAPC"5.1 >= 5.1, [
      AC_DEFINE([HAVE_LUA], [1], [liblua])
      AC_DEFINE([HAVE_LUA_H], [1], [lua.h])
      AC_SUBST(LUA_CFLAGS)
      AC_SUBST(LUA_LIBS)
    ],[
      AC_MSG_RESULT([no])
    ])
  ])
fi

dnl #########################################################################
dnl Check if using jsoncpp
dnl #########################################################################

AC_ARG_WITH(jsoncpp, AC_HELP_STRING([--with-jsoncpp],[Enable JSONCPP for InfluxDB persistence]),
[WITH_JSONCPP=$withval],[WITH_JSONCPP=auto])

if test x"$WITH_JSONCPP" != xno; then
  # try pkgconfig
  if test x"$WITH_JSONCPP" = xyes ; then
    JSONCPPPC=jsoncpp
  elif test x"$WITH_JSONCPP" = xauto ; then
    JSONCPPPC=jsoncpp
  fi

  if test x"$WITH_JSONCPP" = xauto ; then
    PKG_CHECK_MODULES(JSONCPP, $JSONCPPPC >= 1.7.2, [
      AC_SUBST(JSONCPP_CFLAGS)
      AC_SUBST(JSONCPP_LIBS)
      found_jsoncpp=yes
    ],[
      AC_MSG_RESULT([no])
    ])
  else
    PKG_CHECK_MODULES(JSONCPP, $JSONCPPPC >= 1.7.2, [
      AC_SUBST(JSONCPP_CFLAGS)
      AC_SUBST(JSONCPP_LIBS)
      found_jsoncpp=yes
    ])
  fi
fi
AM_CONDITIONAL([JSONCPP], [test x"$found_jsoncpp" = xyes])

if test x"$found_jsoncpp" = xyes ; then
  if test $libcurl_cv_lib_curl_usable = yes ; then
     AC_DEFINE([SUPPORT_INFLUXDB], [1], [Support InfluxDB if both libcurl and jsoncpp are available])
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
                 b64/Makefile
                 b64/src/Makefile
                 include/Makefile
                 test/Makefile
                 conf/Makefile])
AC_OUTPUT
