AC_PREREQ(2.60)
AC_INIT(linknx, 0.0.1.38, cyrille.github@defx.fr)
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

# Check for database support.
AC_DEFINE([MARIADB_DBMS], [2], [Value representing the MariaDB database management system])
AC_DEFINE(MYSQL_DBMS,1,[Value representing the MySQL database management system])
AC_ARG_WITH(database, AC_HELP_STRING([--with-database=[[mysql,mariadb]]],
			[Include support for the given database management system]),
			database=$withval, database=auto)
if test x"$database" != xno -a x"$database" != xauto -a x"$database" != xmariadb -a x"$database" != xmysql ; then
	AC_MSG_ERROR([$database is not a valid value for --with-database])
	database = "no"
fi

AC_ARG_WITH(database-path, AC_HELP_STRING([--with-database-path=/folder/containing/mysql_config], [Path to the utility tool providing build configuration for the selected DBMS. If not set, lookup in PATH]),
			database_path=$withval, database_path="")

if test x"$database" == xauto -o x"$database" == xmariadb ; then
    PKG_CHECK_MODULES(DBMS, mariadb, [
      AC_SUBST(DBMS_CFLAGS)
      AC_SUBST(DBMS_LIBS)
      database="mariadb"
      database_integral_value=2
    ],[
      if test x"$database" == xmariadb ; then
        AC_MSG_ERROR([$database not found])
      fi
    ])
fi

if test x"$database" == xauto -o x"$database" == xmysql ; then
	# If MySQL support was not rejected. Check if it is actually present.
	dbms_config_utility=mysql_config
    if test x"$database_path" != x; then
		# A specific path was provided.
        AC_CHECK_PROG([DBMS_CONFIG],[$dbms_config_utility],[$database_path/$dbms_config_utility],[no],[$database_path])
    else
		# No specific path was provided, lookup in PATH.
        AC_CHECK_PROG([DBMS_CONFIG],[$dbms_config_utility],[$dbms_config_utility],[no])
    fi
    if test "$DBMS_CONFIG" != "no"; then
		# MySQL is actually present.
        DBMS_CFLAGS=`$DBMS_CONFIG --cflags`
        DBMS_LIBS=`$DBMS_CONFIG --libs`
        AC_SUBST(DBMS_CFLAGS)
        AC_SUBST(DBMS_LIBS)
        database="mysql"
        database_integral_value=1
    elif test x"$database" == xmysql ; then
        AC_MSG_ERROR([$database not found])
    else
        database=no
    fi
fi

# Define the HAVE_DBMS macro if either MySQL or MariaDB was selected.
# The value of this macro is respectively 1 or 2 but should be referred to
# using MYSQL_DBMS and MARIADB_DBMS for clarity.
AC_MSG_CHECKING([for database management system])
AC_MSG_RESULT($database)
if test "$database" != "no"; then
	AC_DEFINE_UNQUOTED(HAVE_DBMS,$database_integral_value,[A database management system is available])
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
