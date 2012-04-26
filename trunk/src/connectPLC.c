/*
** Copyright (C) 2012 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "connectPLC.h"

int 
plcConnect (const char *ip, daveConnection **dc, daveInterface **di)
{
  _daveOSserialType fds;
  int j, optval, fail = daveConnectionError;

#ifdef DEBUG
  daveSetDebug (daveDebugAll);
#endif
  // open a TCP on ISO connection with PLC (timeout in seconds)
  fds.rfd = openSocket (102, ip, TIMEOUT);
  fds.wfd = fds.rfd;

  // connect to the PLC
  if (fds.rfd)
    {
      *di = daveNewInterface (fds, "IF1", 0, daveProtoISOTCP, daveSpeed187k);
      daveSetTimeout (*di, TIMEOUT * 1000000);
      *dc = daveNewConnection (*di, MPI_ADDR, RACK, SLOT);
      if (fail = daveConnectPLC (*dc)) 
	closeSocket (fds.rfd);
    }
  return fail;
}

int
plcDisconnect (daveConnection **dc, daveInterface **di)
{
  daveDisconnectPLC(*dc);
  daveDisconnectAdapter(*di);
  return closeSocket ((*di)->fd.rfd);
}
