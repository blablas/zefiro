AC_INIT([zefiro],
        [1.0.0],
        [Emiliano Giovannetti emiliano.giovannetti@tdt.it],
        [zefiro])

AC_CONFIG_AUX_DIR(config)

AC_DEFINE([MPI_ADDR], [2], [MPI's address])
AC_DEFINE([RACK], [0], [Rack number])
AC_DEFINE([SLOT], [3], [CPU's slot])
AC_DEFINE([DAREA], [105], [PLC's data area])
AC_DEFINE([DLEN], [16], [PLC's data area length])
AC_DEFINE([AAREA], [123], [PLC's alarm area])
AC_DEFINE([ALEN], [19], [PLC's alarm area length])
AC_DEFINE([RUN_AS_USER], ["root"], [Daemon's user])
AC_DEFINE([DBHOST], ["localhost"], [PLC's DB host])
AC_DEFINE([DBNAME], ["PLC"], [PLC's DB])
AC_DEFINE([DBUSER], ["plcuser"], [PLC's DB user])
AC_DEFINE([DBPWD], ["plcuser"], [PLC's DB password])
AC_DEFINE([DPERIOD], [1], [PLC's data sampling period (in seconds)])
AC_DEFINE([APERIOD], [120], [PLC's alarm sampling period (in seconds)])

AC_ARG_WITH([timeout], 
		[AS_HELP_STRING([--with-timeout],
		[PLC's connection timeout (in seconds) [default=5]])], 
                [with_timeout=$withval],
                [with_timeout=5])
AC_DEFINE_UNQUOTED([TIMEOUT], [$with_timeout], [PLC's connection timeout (in seconds)])

AC_ARG_ENABLE([debug],
                [AS_HELP_STRING([--enable-debug],
                [enable debug mode (don't use in production) [default=no]])],
                [enable_debug=$enableval],
                [enable_debug=no])
AS_IF([test "$enable_debug" != "no"], [
        AC_DEFINE([DEBUG], [1], ["Debug enabled"])
		  ])

AC_ARG_ENABLE([sampling],
                [AS_HELP_STRING([--enable-sampling],
                [enable sampling mode evaluating anemometer level [default=no]])],
                [enable_sampling=$enableval],
                [enable_sampling=no])
AS_IF([test "$enable_sampling" != "no"], [
        AC_DEFINE([SAMPLING], [1], ["Sampling enabled"])
		  ])
AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])

AC_PROG_CC

AC_CHECK_LIB(nodave, daveGetU16At)

AC_CONFIG_FILES([
   Makefile
   src/Makefile
   test/Makefile
   scripts/Makefile
])
AC_CONFIG_FILES([scripts/zefirod], [chmod o+x scripts/zefirod])
#AC_CONFIG_FILES([scripts/zefirod.check], [chmod o+x scripts/zefirod.check])
#AC_CONFIG_FILES([scripts/zefirod.mail])
#AC_CONFIG_FILES([scripts/zefirod.crontab])

AC_OUTPUT
