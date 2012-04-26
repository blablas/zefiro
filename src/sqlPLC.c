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
#include <string.h>
#include <time.h>
#include "sqlPLC.h"

int
sqlConnect (MYSQL **conn, my_bool autoCommit)
{
  my_bool reconnect = 1;

  if (*conn = mysql_init (NULL))
    {
      //mysql_options(*conn, MYSQL_OPT_RECONNECT, &reconnect);
      if (!(mysql_real_connect (*conn,
			      DBHOST,		 
			      DBUSER,		 
			      DBPWD,		 
			      DBNAME,			 
			      0,			 
			      NULL,			 
			      0)))
	if (!(mysql_autocommit (*conn, autoCommit)))
	  return 0;
    }
  return mysql_errno (*conn);
}

void
sqlDisconnect (MYSQL **conn)
{
  mysql_close (*conn);
}

int 
sqlCloseStmt (MYSQL_STMT **stmt)
{
  mysql_stmt_free_result(*stmt);
  if (mysql_stmt_close (*stmt))
    return mysql_stmt_errno (*stmt);
  else return 0;
}

int
sqlPrepareStmt (MYSQL *conn, MYSQL_STMT **stmt, const char *query)
{
  if (*stmt = mysql_stmt_init (conn))
    if (mysql_stmt_prepare (*stmt, query, strlen (query)))
      return mysql_stmt_errno (*stmt);
    else return 0;
  return mysql_stmt_errno (*stmt);
}

int
sqlBindParam (MYSQL_STMT *stmt, MYSQL_BIND *bind)
{
  if (mysql_stmt_bind_param (stmt, bind))
    return mysql_stmt_errno (stmt);
  else return 0;
}

int
sqlBindResult (MYSQL_STMT *stmt, MYSQL_BIND *bind)
{
  if (mysql_stmt_bind_result (stmt, bind))
    return mysql_stmt_errno (stmt);
  else return 0;
}

int sqlExecStmt (MYSQL_STMT *stmt)
{
  if (!mysql_stmt_execute (stmt))
    if (mysql_stmt_store_result (stmt))
      return mysql_stmt_errno (stmt);
    else return 0;
  return mysql_stmt_errno (stmt);
}
