# Copyright (C) 2012 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

sbin_PROGRAMS = test01 test02
test01_SOURCES = test01.c $(top_srcdir)/src/connectPLC.c
test01_CPPFLAGS = -I$(top_srcdir)/include -DLINUX
test02_SOURCES = test02.c $(top_srcdir)/src/statePLC.c
test02_CPPFLAGS = -I$(top_srcdir)/include -DLINUX
