# Copyright (C) 2009 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

initdir = /etc/init.d
crondir = /etc/cron.d
cron_DATA = zefirod.crontab
sbin_SCRIPTS = zefirod.check
noinst_SCRIPTS = zefirod 
sysconf_DATA = zefirod.mail
CLEANFILES = $(sbin_SCRIPTS) $(noinst_SCRIPTS) $(sysconf_DATA) $(cron_DATA)

install-exec-local:
	cp -af $(noinst_SCRIPTS) $(initdir) && \
		chkconfig --add $(noinst_SCRIPTS) && \
		chkconfig $(noinst_SCRIPTS) on

uninstall-local:
	chkconfig --del $(noinst_SCRIPTS) && \
		rm -f $(initdir)/$(noinst_SCRIPTS) 
