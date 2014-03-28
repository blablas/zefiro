#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "statePLC.h"
#include "sqlPLC.h"

typedef struct data * dataPtr;
struct data {
    int min,
	max,
	avg,
	first,
	last;
    int vList[10];
};

void
processData (dataPtr plc, const int value)
{
  int i;

  plc->min = 1000;
  plc->max = -1000;
  plc->avg = 0;
  // new value for last index
  plc->last = (plc->last + 1) % 10; 
  if (plc->vList[plc->first] >= 0)
    // MAX element queue 
    if (plc->last == plc->first) 
      // new value form first index
      plc->first = (plc->first + 1) % 10;
  // insert value
  plc->vList[plc->last] = value;
  if (plc->vList[(plc->last + 1) % 10] >= 0)
    {
      // Calculate min, max, avg
      for (i = 0; i < 10; i++)
	{
	  printf ("%d) %d\n", (plc->first + i) % 10 , plc->vList[(plc->first + i) % 10]);
	  if (plc->vList[i] < plc->min)
	    plc->min = plc->vList[i];
	  if (plc->vList[i] > plc->max)
	    plc->max = plc->vList[i];
	  plc->avg += plc->vList[i];
	}
      plc->avg /= 10;
    }
}

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
      evt,
      ntv,
      smp,
      hys,
      lvl,
      ret;
  dataPtr dta;
  stateLst pos = A, rows;
  MYSQL *conn;
  MYSQL_STMT *stmt;
  MYSQL_BIND paramU[6], 
	     resultSet[11];

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
  // 'evt'
  resultSet[8].buffer_type = MYSQL_TYPE_LONG;
  resultSet[8].buffer = (void *)&evt;
  resultSet[8].is_null = 0;
  resultSet[8].is_unsigned = 0; 
  // 'ntv'
  resultSet[9].buffer_type = MYSQL_TYPE_LONG;
  resultSet[9].buffer = (void *)&ntv;
  resultSet[9].is_null = 0;
  resultSet[9].is_unsigned = 0; 
  // 'smp'
  resultSet[10].buffer_type = MYSQL_TYPE_LONG;
  resultSet[10].buffer = (void *)&smp;
  resultSet[10].is_null = 0;
  resultSet[10].is_unsigned = 0; 
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
      add2StateDscTbl (id, vp, vmax, vmin, iv, sup, low, evt, ntv, smp);
      printf ("level: id(%d), vp(%d), vmax(%d), vmin(%d), iv(%d), up(%d), do(%d), hys(%d), evt(%d), ntv(%d), smp(%d), fun(%d)\n", 
	      id, vp, vmax, vmin, iv, sup, low, hys, evt, ntv, smp, id); 
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
  id = 1;
  dta = (dataPtr) malloc (sizeof (struct data));

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
  paramU[1].buffer = (void *)&(dta->max);
  paramU[1].is_null = 0;
  paramU[1].is_unsigned = 0;
  // 'vmin'
  paramU[2].buffer_type = MYSQL_TYPE_TINY;
  paramU[2].buffer = (void *)&(dta->min);
  paramU[2].is_null = 0;
  paramU[2].is_unsigned = 0;
  // 'iv'
  paramU[3].buffer_type = MYSQL_TYPE_TINY;
  paramU[3].buffer = (void *)&(dta->avg);
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

  dta->first = 0;
  dta->last = 9;
  printf ("('q' to quit): ");
//  while (scanf ("%d", &value))
  while (getchar () != 'q')
    {
//      value = 1 + (int) (100.0 * (rand() / (RAND_MAX + 1.0)));
      value = ((double) rand() / (RAND_MAX + 1.0)) * (80 - 10 + 1) + 10;
      processData(dta, value);
      printf ("processed data: vp(%d), vmax(%d), vmin(%d), iv(%d)\n", value, dta->max, dta->min, dta->avg);
      lvl = actual->state->newState (actual, value, dta->max, dta->min, dta->avg);
      if (sqlExecStmt (stmt))
	{
	  printf ("sqlExecStmt (U): %d\n", res);
	  return res;
	}
//      printf ("effected rows: %d\n", mysql_stmt_affected_rows (stmt));
      printf ("new state: level(%d), up counter(%d), down counter(%d)\n", lvl, actual->ucount, actual->dcount); 
      printf ("('q' to quit): ");
    }

  if (res = sqlCloseStmt (&stmt))
    {
      printf ("sqlCloseStmt: %d\n", res);
      return res;
    }
  sqlDisconnect (&conn);

  free (dta);
  disposeActualState (actual);
  resetStateDscTbl ();

  return 0;
}
