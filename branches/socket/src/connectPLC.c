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

//int 
//plcConnect (const char *ip, int mpi, int rack, int slot, daveConnection **dc)
//{
//  _daveOSserialType fds;
//  int j, 
//      optval, 
//      res = daveConnectionError;
//  daveInterface *di;
//
//  // open a TCP on ISO connection with PLC (timeout in seconds)
//  fds.rfd = openSocket (102, ip, TIMEOUT);
//  fds.wfd = fds.rfd;
//
//  // connect to the PLC
//  if (fds.rfd)
//    {
//      di = daveNewInterface (fds, "IF1", 1, daveProtoISOTCP, daveSpeed187k);
//      daveSetTimeout (di, TIMEOUT * 1000000);
//      if (*dc = daveNewConnection (di, mpi, rack, slot))
//	//if (res = daveConnectPLC (*dc)) 
//	  // plcDisconnect (*dc);
//	res = daveConnectPLC (*dc);
//    }
//  //else 
//    //*dc = NULL;
//  return res;
//}

daveConnection* 
plcConnect (const char *ip, int mpi, int rack, int slot, int *err)
{
  _daveOSserialType fds;
  int j, 
      optval; 
  daveInterface *di;
  daveConnection *dc = NULL;

  *err = daveConnectionError;
  // open a TCP on ISO connection with PLC (timeout in seconds)
  fds.rfd = openSocket (102, ip, TIMEOUT);
  fds.wfd = fds.rfd;

  // connect to the PLC
  if (fds.rfd)
    {
      di = daveNewInterface (fds, "IF1", 1, daveProtoISOTCP, daveSpeed187k);
      daveSetTimeout (di, TIMEOUT * 1000000);
      if (dc = daveNewConnection (di, mpi, rack, slot))
	if (*err = daveConnectPLC (dc))
	  if (plcDisconnect (dc))
	    *err = daveConnectionError;
    }
  return dc;
}

int
plcDisconnect (daveConnection *dc)
{
  int res = 0;

  if (dc)
    {
      daveDisconnectPLC (dc);				// dummy function, always returns 0
      if (dc->iface)
	{
	  daveDisconnectAdapter (dc->iface);		// dummy function, always returns 0
	  if (dc->iface->fd.rfd)
	    res = closeSocket (dc->iface->fd.rfd);	// close socket (0 on success, -1 on failure)
	  free (dc->iface);
	  dc->iface = NULL;
	}
      free (dc);
    }
  return res;
}
