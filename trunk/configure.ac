AC_INIT([zefiro],
        [1.0.0],
        [Emiliano Giovannetti emiliano.giovannetti@tdt.it],
        [zefiro])
AC_CONFIG_AUX_DIR(config)
#AC_ARG_ENABLE(test,
#                [AS_HELP_STRING([--enable-test],
#                [enable test mode (don't use in production) [default=no]])],
#                [enable_test=$enableval],
#                [enable_test=no])
#
#if test "$enable_test" = "yes"; then
#        AC_DEFINE([DB_NAME], ["firebird01:y:/data/sms/test/labsms.fdb"], [Test DB server])
#        AC_DEFINE([OK_SUBJECT], ["Test mail OK"], [Test subject for 'Message sent' mail])
#        AC_DEFINE([KO_SUBJECT], ["Test mail not OK"], [Test subject for 'Message not sent' mail])
#       AC_DEFINE([ERR_SUBJECT], ["Send SMS error"], [Subject for 'Message error' mail])
#        AC_DEFINE([RCPT_TO], ["emiliano.giovannetti@tdt.it"], [Test TO recipient])
#        AC_DEFINE([CC_TO], ["emiliano.giovannetti@tdt.it"], [Test CC recipient])
#        AC_DEFINE([MAIL_FROM], ["smtpuser@tdt.it"], [Test MAIL FROM recipient])
#        AC_DEFINE([PROXY_HOST], ["10.50.1.15"], [Test PROXY HOST])
#        AC_DEFINE([PROXY_PORT], [3128], [Test PROXY PORT])
#else
	#AC_DEFINE([DB_NAME], ["firebird01:y:/data/sms/wrk/labsms.fdb"], [Production DB server])
	#AC_DEFINE([OK_SUBJECT], ["SMS Messages sent succesfully"], [Subject for 'Message sent' mail])
	#AC_DEFINE([KO_SUBJECT], ["SMS Messages trasmission error"], [Subject for 'Message not sent' mail])
	#AC_DEFINE([ERR_SUBJECT], ["Send SMS error"], [Subject for 'Message error' mail])
	#AC_DEFINE([RCPT_TO], ["avviamentopersonale@tdt.it"], [TO recipient])
	#AC_DEFINE([CC_TO], ["IT.Department@tdt.it"], [CC recipient])
	#AC_DEFINE([MAIL_FROM], ["smtpuser@tdt.it"], [MAIL FROM recipient])
	#AC_DEFINE([PROXY_HOST], ["10.50.1.15"], [PROXY HOST])
	#AC_DEFINE([PROXY_PORT], [3128], [PROXY PORT])
#fi
AC_DEFINE([RUN_AS_USER], ["root"], [Daemon's user])

AC_ARG_ENABLE(debug,
                [AS_HELP_STRING([--enable-debug],
                [enable debug mode (don't use in production) [default=no]])],
                [enable_debug=$enableval],
                [enable_debug=no])

if test "$enable_debug" = "yes"; then
        AC_DEFINE([DEBUG], [1], ["Enables debug"])
fi

AC_DEFINE([DB_USER], ["zefiro_user"], [Sqlite db user])
AC_DEFINE([DB_PASSWORD], ["masterkey"], [Sqlite db password])

AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])

AC_PROG_CC
#LF_CONFIGURE_CC
#LF_SET_WARNINGS

#AC_CHECK_LIB(gds, isc_attach_database)
#AC_CHECK_LIB(glib, main)

AC_CONFIG_FILES([
   Makefile
   src/Makefile
])

AC_OUTPUT

