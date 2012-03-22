AC_INIT([zefiro],
        [1.0.0],
        [Emiliano Giovannetti emiliano.giovannetti@tdt.it],
        [zefiro])

AC_CONFIG_AUX_DIR(config)

AC_DEFINE([MPI_ADDR], [2], [MPI's address])
AC_DEFINE([RACK], [0], [Rack number])
AC_DEFINE([SLOT], [2], [CPU's slot])
AC_DEFINE([DAREA], [105], [PLC's data area])
AC_DEFINE([DBLEN], [50], [PLC's data area length])
AC_DEFINE([RUN_AS_USER], ["root"], [Daemon's user])
AC_DEFINE([DBHOST], ["localhost"], [PLC's DB host])
AC_DEFINE([DBNAME], ["PLC"], [PLC's DB])
AC_DEFINE([DBUSER], ["plcuser"], [PLC's DB user])
AC_DEFINE([DBPWD], ["plcuser"], [PLC's DB password])

AC_ARG_WITH([timeout], 
		[AS_HELP_STRING([--with-timeout],
		[PLC's connection timeout (in seconds) [default=5]])], 
                [with_timeout=$withval],
                [with_timeout=5])
AC_DEFINE_UNQUOTED([TIMEOUT], [$with_timeout], [PLC's connection timeout (in seconds)])

AC_ARG_WITH([max-samples],
                [AS_HELP_STRING([--with-max-samples],
                [Max sampling interval for media calculation [default=10]])],
                [with_max=$withval],
                [with_max=10])
AC_DEFINE_UNQUOTED([MAX], [$with_max], [Max sampling interval for media calculation])

AC_ARG_WITH([min-samples],
                [AS_HELP_STRING([--with-min-samples],
                [Min sampling interval for media calculation [default=5]])],
                [with_min=$withval],
                [with_min=5])
AC_DEFINE_UNQUOTED([MIN], [$with_min], [Min sampling interval for media calculation])

AC_ARG_ENABLE([debug],
                [AS_HELP_STRING([--enable-debug],
                [enable debug mode (don't use in production) [default=no]])],
                [enable_debug=$enableval],
                [enable_debug=no])
AS_IF([test "$enable_debug" != "no"], [
        AC_DEFINE([DEBUG], [1], ["Debug enabled"])
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
AC_CONFIG_FILES([scripts/zefirod.check], [chmod o+x scripts/zefirod.check])
AC_CONFIG_FILES([scripts/zefirod.mail])
AC_CONFIG_FILES([scripts/zefirod.crontab])

AC_OUTPUT
