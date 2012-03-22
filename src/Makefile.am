# Copyright (C) 2012 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

sbin_PROGRAMS = zefirod
zefirod_SOURCES = zefiro.c
zefirod_CPPFLAGS = -I$(top_srcdir)/include -I/usr/include/mysql -DLINUX
zefirod_LDADD = -L/usr/lib64/mysql -lmysqlclient_r -lpthread -lm -lrt -lssl -lcrypto -ldl
