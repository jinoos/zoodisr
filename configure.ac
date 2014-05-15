m4_define([ZR_NAME], [zoodisr])
m4_define([ZR_MAJOR], 0)
m4_define([ZR_MINOR], 1)
m4_define([ZR_PATCH], 0)
m4_define([ZR_BUGS], [jinoos@gmail.com])

AC_PREREQ([2.00])
AC_INIT(ZR_NAME, [ZR_MAJOR.ZR_MINOR.ZR_PATCH], ZR_BUGS)
AM_INIT_AUTOMAKE(ZR_NAME, [ZR_MAJOR.ZR_MINOR.ZR_PATCH])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

AC_DEFINE(ZR_VERSION_MAJOR, ZR_MAJOR, [Define the major version number])
AC_DEFINE(ZR_VERSION_MINOR, ZR_MINOR, [Define the minor version number])
AC_DEFINE(ZR_VERSION_PATCH, ZR_PATCH, [Define the patch version number])
AC_DEFINE(ZR_VERSION_STRING, "ZR_MAJOR.ZR_MINOR.ZR_PATCH", [Define the version string])

AC_LANG([C])

# Checks for libraries.
AC_CHECK_LIB([m],[floorf],[],[AC_MSG_ERROR([math library required.])])

##########################################################################

DEFAULT_LIBEVENT_PATH=/usr
AC_ARG_WITH([libevent],
    [AS_HELP_STRING([--with-libevent=PATH],
        [set specific libevent installation path.])],
    [],
    [with_libevent=default])

if test "x$with_libevent" == "xdefault" || test "x$with_libevent" == "x"; then
    PKG_CHECK_MODULES([libevent], [libevent >= 2.0])
    LIBS="$LIBS $libevent_LIBS"
else
    DEFAULT_LIBEVENT_PATH=$with_libevent

    AC_CHECK_FILES([${DEFAULT_LIBEVENT_PATH}/include/event2/event.h ${DEFAULT_LIBEVENT_PATH}/lib/libevent.so],,
                    [AC_MSG_ERROR([Cannot find libevent header and library under ${DEFAULT_LIBEVENT_PATH}])])

    CFLAGS="$CFLAGS -I${DEFAULT_LIBEVENT_PATH}/include"
    LDFLAGS="$LDFLAGS -L${DEFAULT_LIBEVENT_PATH}/lib"
    LIBS="$LIBS -levent"
fi

##########################################################################
DEFAULT_ZOOKEEPER_PATH=/usr
AC_ARG_WITH([zookeeper],
    [AS_HELP_STRING([--with-zookeeper=PATH],
        [set specific zookeeper installation path.])],
    [],
    [with_zookeeper=default])

if test "x$with_zookeeper" == "xdefault" || test "x$with_zookeeper" == "x"; then
    AC_CHECK_LIB([zookeeper_mt],[getenv],[],[AC_MSG_ERROR([Please install zookeeper library (multithread devel)])])
    AC_CHECK_HEADERS([zookeeper/zookeeper.h],[],[AC_MSG_ERROR([Please install zookeeper library (multithread devel)])])
else
    DEFAULT_ZOOKEEPER_PATH=$with_zookeeper

    AC_CHECK_FILES([${DEFAULT_ZOOKEEPER_PATH}/include/zookeeper/zookeeper.h ${DEFAULT_ZOOKEEPER_PATH}/lib/libzookeeper_mt.so],,
                    [AC_MSG_ERROR([Cannot find zookeeper header and library under ${DEFAULT_ZOOKEEPER_PATH}])])

    CFLAGS="$CFLAGS -I${DEFAULT_ZOOKEEPER_PATH}/include"
    LDFLAGS="$LDFLAGS -L${DEFAULT_ZOOKEEPER_PATH}/lib"
fi

##########################################################################
AC_ARG_WITH([zookeeper-static],
    [AS_HELP_STRING([--with-zookeeper-static=LIBRARY_ARCHIVE_PATH],
        [path of zookeeper library archive file as libzookeeper_mt.a ])],
    [AC_CHECK_FILES([${with_zookeeper_static}],
        [AC_MSG_NOTICE([found zookeeper library archive ${with_zookeeper_static}])],
        [AC_MSG_ERROR([Cannot find library archive file ${with_zookeeper_static}])])],
    [with_zookeeper_static=default])

if test "x$with_zookeeper_static" == "xdefault"; then
    LIBS="$LIBS -lzookeeper_mt"
else
    LDFLAGS="$with_zookeeper_static $LDFLAGS"
    LIBS="$LIBS -lm -lpthread"
fi


# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memmove memset strchr strtol])


#########################################################################

#
# for Multi-thread application
#
#AC_DEFINE([LOG_FILELINE], [1], [For show filename and line number in log])

#
# for Multi-thread application
#
#AC_DEFINE([DEBUG], [1], [For debugging])

#
# for Multi-thread application
#
AC_DEFINE([THREAD_SAFE], [1], [For multi-thread application])

#
# for ATOMIC defination.
#
AH_BOTTOM([
#if (__i386 || __amd64) && __GNUC__
#define GNUC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GNUC_VERSION >= 40100
#define HAVE_ATOMIC
#endif
#endif
])

AC_CONFIG_FILES([Makefile
                    src/Makefile
                    test/Makefile
                    ])

AC_OUTPUT
