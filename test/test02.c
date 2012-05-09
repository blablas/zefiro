#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "statePLC.h"
#include "sqlPLC.h"

int 
main (void)
{
  int res,
      value = 0, 
      // SELECT_ENABLE_LEVELS output buffers and UPDATE_LIVE_DTA input parameters
      id, 
      vp,
      vmax,
      vmin,
      iv,
      sup, 
      low,
      hys,
      lvl,
      ret;
  stateLst pos = A, rows;
  MYSQL *conn;
  MYSQL_STMT *stmt;
  MYSQL_BIND paramU[6], 
	     resultSet[8];

  if (res = sqlConnect (&conn, 1))
    {
      printf ("sqlConnect: %d\n", res);
      return res;
    }
  if (res = sqlPrepareStmt (conn, &stmt, SELECT_ENABLE_LEVELS))
    {
      printf ("sqlPrepareStmt: %d\n", res);
      return res;
    }
  // Set output buffers
  memset (resultSet, 0, sizeof (resultSet));
  // actual 'id' 
  resultSet[0].buffer_type = MYSQL_TYPE_LONG;
  resultSet[0].buffer = (void *)&id;
  resultSet[0].is_null = 0;
  resultSet[0].is_unsigned = 0;
  // 'vp'
  resultSet[1].buffer_type = MYSQL_TYPE_LONG;
  resultSet[1].buffer = (void *)&vp;
  resultSet[1].is_null = 0;
  resultSet[1].is_unsigned = 0;
  // 'vmax'
  resultSet[2].buffer_type = MYSQL_TYPE_LONG;
  resultSet[2].buffer = (void *)&vmax;
  resultSet[2].is_null = 0;
  resultSet[2].is_unsigned = 0;
  // 'vmin'
  resultSet[3].buffer_type = MYSQL_TYPE_LONG;
  resultSet[3].buffer = (void *)&vmin;
  resultSet[3].is_null = 0;
  resultSet[3].is_unsigned = 0;
  // 'iv'
  resultSet[4].buffer_type = MYSQL_TYPE_LONG;
  resultSet[4].buffer = (void *)&iv;
  resultSet[4].is_null = 0;
  resultSet[4].is_unsigned = 0;
  // 'up' threshold
  resultSet[5].buffer_type = MYSQL_TYPE_LONG;
  resultSet[5].buffer = (void *)&sup;
  resultSet[5].is_null = 0;
  resultSet[5].is_unsigned = 0;
  // 'do' threshold
  resultSet[6].buffer_type = MYSQL_TYPE_LONG;
  resultSet[6].buffer = (void *)&low;
  resultSet[6].is_null = 0;
  resultSet[6].is_unsigned = 0; 
  // 'hys' threshold
  resultSet[7].buffer_type = MYSQL_TYPE_LONG;
  resultSet[7].buffer = (void *)&hys;
  resultSet[7].is_null = 0;
  resultSet[7].is_unsigned = 0; 
  if (res = sqlBindResult (stmt, resultSet))
    {
      printf ("sqlBindParam: %d\n", res);
      return res;
    }
  if (sqlExecStmt (stmt))
    {
      printf ("sqlExecStmt: %d\n", res);
      return res;
    }
  // initialize actual state table
  rows = mysql_stmt_num_rows (stmt);
  printf ("effected rows: %d\n", rows);
  while (!mysql_stmt_fetch (stmt))
    {
      if (pos == (rows - 1))
	pos = E;
      add2StateDscTbl (pos, id, vp, sup, low);	// state level: could be one among vp, vmax, vmin, iv
      printf ("level: id(%d), vp(%d), up(%d), do(%d), hys(%d), fun(%d)\n", id, vp, sup, low, hys, pos); 
      pos++;
    }
  if (res = sqlCloseStmt (&stmt))
    {
      printf ("sqlCloseStmt: %d\n", res);
      return res;
    }

  // initialize state machine in state A
  actualStatePtr actual;
  actual = initActualState (A);

  // update liveValue in PLC DB for Test PLC (id = 1)
  if (res = sqlPrepareStmt (conn, &stmt, UPDATE_LIVE_DTA))
    {
      printf ("sqlPrepareStmt: %d\n", res);
      return res;
    }
  // set input parameters
  memset (paramU, 0, sizeof (paramU));
  // 'vp' 
  paramU[0].buffer_type = MYSQL_TYPE_TINY;
  paramU[0].buffer = (void *)&value;
  paramU[0].is_null = 0;
  paramU[0].is_unsigned = 0;
  // 'vmax'
  paramU[1].buffer_type = MYSQL_TYPE_TINY;
  paramU[1].buffer = (void *)&vmax;
  paramU[1].is_null = 0;
  paramU[1].is_unsigned = 0;
  // 'vmin'
  paramU[2].buffer_type = MYSQL_TYPE_TINY;
  paramU[2].buffer = (void *)&vmin;
  paramU[2].is_null = 0;
  paramU[2].is_unsigned = 0;
  // 'iv'
  paramU[3].buffer_type = MYSQL_TYPE_TINY;
  paramU[3].buffer = (void *)&iv;
  paramU[3].is_null = 0;
  paramU[3].is_unsigned = 0;
  // 'lvl'
  paramU[4].buffer_type = MYSQL_TYPE_TINY;
  paramU[4].buffer = (void *)&lvl;
  paramU[4].is_null = 0;
  paramU[4].is_unsigned = 0;
  // 'id' 
  paramU[5].buffer_type = MYSQL_TYPE_TINY;
  paramU[5].buffer = (void *)&id;
  paramU[5].is_null = 0;
  paramU[5].is_unsigned = 0;
  if (res = sqlBindParam (stmt, paramU))
    {
      printf ("sqlBindPram (U): %d\n", res);
      return res;
    }

  // LIVEQ input parameters
  vmax = 0,	// SHOULD BE CALCULATED
  vmin = 0,	// SHOULD BE CALCULATED
  iv = 0;	// SHOULD BE CALCULATED 
  id = 1;	// PLC's index
  printf ("value ('q' to quit): ");
  while (scanf ("%d", &value))
    {
      lvl = actual->state->newState (actual, value);
      if (sqlExecStmt (stmt))
	{
	  printf ("sqlExecStmt (U): %d\n", res);
	  return res;
	}
      printf ("effected rows: %d\n", mysql_stmt_affected_rows (stmt));
      printf ("new state: level(%d), up counter(%d), down counter(%d)\n", lvl, actual->ucount, actual->dcount); 
      printf ("new alarm: level(%d), down counter(%d)\n", actual->alarm, actual->alevel); 
      printf ("value ('q' to quit): ");
    }

  if (res = sqlCloseStmt (&stmt))
    {
      printf ("sqlCloseStmt: %d\n", res);
      return res;
    }
  sqlDisconnect (&conn);

  disposeActualState (actual);
  resetStateDscTbl ();

  return 0;
}
