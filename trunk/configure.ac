AC_INIT([zefiro],
        [1.0.0],
        [Emiliano Giovannetti emiliano.giovannetti@tdt.it],
        [zefiro])
AC_CONFIG_AUX_DIR(config)

AC_DEFINE([MPI_ADDR], [2], [MPI's address])
AC_DEFINE([RACK], [0], [Rack number])
AC_DEFINE([SLOT], [2], [CPU's alot])
AC_DEFINE([TIMEOUT], [5000000], [Connection's timeout])
AC_DEFINE([RUN_AS_USER], ["root"], [Daemon's user])
AC_DEFINE([IP_BASE_ADDR], ["10.50.3.22"], [PLC's base ip address])
AC_DEFINE([NPLC], [1], [Number of PLCs])

AC_ARG_ENABLE(debug,
                [AS_HELP_STRING([--enable-debug],
                [enable debug mode (don't use in production) [default=no]])],
                [enable_debug=$enableval],
                [enable_debug=no])

if test "$enable_debug" = "yes"; then
        AC_DEFINE([DEBUG], [1], ["Debug enabled"])
fi

AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])

AC_PROG_CC
#LF_CONFIGURE_CC
#LF_SET_WARNINGS

AC_CHECK_LIB(nodave, daveReadBytes)

AC_CONFIG_FILES([
   Makefile
   src/Makefile
   test/Makefile
])

AC_OUTPUT

