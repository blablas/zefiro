# Copyright (C) 2012 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_PROGRAMS = test01 test02 test03
test01_SOURCES = test01.c $(top_srcdir)/src/connectPLC.c
test01_CPPFLAGS = -I$(top_srcdir)/include -DLINUX
test02_SOURCES = test02.c $(top_srcdir)/src/statePLC.c $(top_srcdir)/src/sqlPLC.c
test02_CPPFLAGS = -I$(top_srcdir)/include -I/usr/include/mysql -DLINUX
test02_LDADD = -L/usr/lib64/mysql -lmysqlclient_r -lpthread -lm -lrt -lssl -lcrypto -ldl
test03_SOURCES = test03.c $(top_srcdir)/src/threadPLC.c
test03_CPPFLAGS = -I$(top_srcdir)/include -DLINUX
test03_LDADD = -lpthread -lrt
