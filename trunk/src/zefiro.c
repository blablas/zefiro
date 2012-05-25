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
#include "zefiro.h"

int lfp = -1;
char *pidfile = NULL;
unsigned int LPARAM, 
	     MIN, 
	     MAX, 
	     NRETRY;

void
daemonize (const char *pidfile)
{
  pid_t pid, 
	sid, 
	parent;
  sigset_t waitMask,
	   childMask;
  int rsig;
  char cpid[5] = { 0x00 };

  // block all signals (inherited by threads)
  sigfillset (&waitMask);
  sigprocmask (SIG_BLOCK, &waitMask, NULL);

  // Already a daemon 
  if (getppid () == 1)
    return;

  // Create the pid file as the current user 
  if (pidfile && pidfile[0])
    {
      lfp = open (pidfile, O_RDWR | O_CREAT, 0640);
      if (lfp < 0)
	{
	  syslog (LOG_ERR, "unable to create pid file %s, code=%d (%s)",
		  pidfile, errno, strerror (errno));
	  exit (EXIT_FAILURE);
	}
    }

  // mask for exit signals
  sigemptyset (&childMask);
  sigaddset (&childMask, SIGCHLD);
  sigaddset (&childMask, SIGTERM);
  // Fork off the parent process 
  pid = fork ();
  if (pid < 0)
    {
      syslog (LOG_ERR, "unable to fork daemon, code=%d (%s)",
	      errno, strerror (errno));
      exit (EXIT_FAILURE);
    }
  // If we got a good PID, then we can exit the parent process. 
  if (pid > 0)
    {
      sprintf (cpid, "%d", pid);
      if (write (lfp, &cpid, sizeof (cpid)) < 0)
	{
	  syslog (LOG_ERR, "unable to write the pid file %s, code=%d (%s)",
		  pidfile, errno, strerror (errno));
	  exit (EXIT_FAILURE);
	}
      free ((void *) pidfile);
      close (lfp);
      // wait for termination signals (SIGTERM, SIGCHLD)
      sigwait (&childMask, &rsig);
      exit (EXIT_SUCCESS);
    }

  // At this point we are executing as the child process 
  parent = getppid ();

  // Change the file mode mask 
  umask (0);

  // Create a new SID for the child process 
  sid = setsid ();
  if (sid < 0)
    {
      syslog (LOG_ERR, "unable to create a new session, code %d (%s)",
	      errno, strerror (errno));
      exit (EXIT_FAILURE);
    }
  // Change the current working directory.  This prevents the current
  // directory from being locked; hence not being able to remove it. 
  if ((chdir ("/")) < 0)
    {
      syslog (LOG_ERR, "unable to change directory to %s, code %d (%s)",
	      "/", errno, strerror (errno));
      exit (EXIT_FAILURE);
    }

  // Redirect standard files to /dev/null 
  freopen ("/dev/null", "r", stdin);
  freopen ("/dev/null", "w", stdout);
  freopen ("/dev/null", "w", stderr);

  kill (parent, SIGCHLD);

  // Drop user if there is one, and we were run as root 
  if (getuid () == 0 || geteuid () == 0)
    {
      struct passwd *pw = getpwnam (RUN_AS_USER);
      if (pw)
	setuid (pw->pw_uid);
    }
}

void
processData (pDataPtr plc, const int value)
{
  int i;

  plc->min = INT_MAX;
  plc->max = INT_MIN;
  plc->avg = 0;
  // new value for last index
  plc->last = (plc->last + 1) % MAX; 
  if (plc->vList[plc->first] >= 0)
    // MAX element queue 
    if (plc->last == plc->first) 
      // new value form first index
      plc->first = (plc->first + 1) % MAX;
  // insert value
  plc->vList[plc->last] = value;
  if (plc->vList[(plc->last + 1) % MAX] >= 0)
    {
      // Calculate min, max, avg
      for (i = 0; i < MAX; i++)
	{
	  if (plc->vList[i] < plc->min)
	    plc->min = plc->vList[i];
	  if (plc->vList[i] > plc->max)
	    plc->max = plc->vList[i];
	  plc->avg += plc->vList[i];
#ifdef DEBUG
	  syslog (LOG_INFO, "VALUE(%d): %d, MIN: %d, MAX: %d", (plc->first + i) % MAX, plc->vList[(plc->first + i) % MAX], plc->min, plc->max); 
#endif
	}
      plc->avg /= MAX;
#ifdef DEBUG
      syslog (LOG_INFO, "AVG: %d", plc->avg); 
#endif
    }
}

pDataPtr*
initPlcsData (MYSQL *conn, int *len)
{
  pDataPtr *dta;
  MYSQL_STMT *stmt;
  MYSQL_BIND resultSet[2];
  // SELECT_PLC_DTA output buffers
  char ip[15];
  int id, 
      i,
      res = 0,
      pos = 0;

  if (res = sqlPrepareStmt (conn, &stmt, SELECT_PLC_DTA))
    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else
    {
      // Set output buffers for registry resultset
      memset (resultSet, 0, sizeof (resultSet));
      // 'id' 
      resultSet[0].buffer_type = MYSQL_TYPE_LONG;
      resultSet[0].buffer = (void *)&id;
      resultSet[0].is_null = 0;
      resultSet[0].is_unsigned = 0;
      // 'ip'
      resultSet[1].buffer_type = MYSQL_TYPE_STRING;
      resultSet[1].buffer = (void *)ip;
      resultSet[1].buffer_length = sizeof (ip);
      resultSet[1].is_null = 0;
      if (res = sqlBindResult (stmt, resultSet))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	{	
	  if (res = sqlExecStmt (stmt))
	    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
	  else 
	    {
	      // initialize PLCs' data (id, ip)
	      *len = mysql_stmt_num_rows (stmt);
	      dta = (pDataPtr *) malloc (*len * sizeof (*dta));
	      while (!mysql_stmt_fetch (stmt))
		{
		  if (dta[pos] = (pDataPtr) malloc (sizeof (**dta)))
		    {
		      dta[pos]->id = id;
		      strcpy (dta[pos]->ip, ip);
		      dta[pos]->sig = SIGRTMIN + pos;		    
		      dta[pos]->vList = (int *) malloc (MAX * sizeof (int));
		      for (i = 0; i < MAX; i++)
			{
			  dta[pos]->vList[i] = -1;
#ifdef DEBUG
			  syslog (LOG_INFO, "dta[%d]->vList[%d]: %d", pos, i, dta[pos]->vList[i]);
#endif
			}
		      for (i = 0; i < sizeof (dta[pos]->alm); i++)
			dta[pos]->alm[i] = (unsigned char) 0;
		      for (i = 0; i < sizeof (dta[pos]->nfo); i++)
			dta[pos]->nfo[i] = (unsigned char) 0;
		      dta[pos]->first = 0;
		      dta[pos]->last = MAX - 1; 
		      dta[pos]->vp1 = 0;
		      dta[pos]->vp2 = 0;
		      dta[pos]->wgt = 0;
		      dta[pos]->hgt = 0;
		      dta[pos]->min = INT_MAX;
		      dta[pos]->max = INT_MIN;
		      dta[pos]->avg = 0.0;
		      // initialize activity state to level A
		      dta[pos]->act = initActualState (A);  
		    }
		  else
		    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, strerror (errno));
		  pos++;
		}
	    }
	}
      if (sqlCloseStmt (&stmt))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
    }
  return dta;
}

void 
freePlcsData (pDataPtr *dta, const int len)
{
  int i;

  for (i = 0; i < len; i++)
    {
      free (dta[i]->vList);
      disposeActualState (dta[i]->act);
      free (dta[i]);
    }
  free (dta);
}

int
initConfParams (MYSQL *conn)
{
  MYSQL_STMT *stmt;
  MYSQL_BIND resultSet[4];
  int res = 0;

  if (res = sqlPrepareStmt (conn, &stmt, GET_CONF_PARAMS))
    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else
    {
      // Set output buffers for configParams resultset
      memset (resultSet, 0, sizeof (resultSet));
      // 'MIN' 
      resultSet[0].buffer_type = MYSQL_TYPE_LONG;
      resultSet[0].buffer = (void *)&MIN;
      resultSet[0].is_null = 0;
      resultSet[0].is_unsigned = 1;
      // 'MAX' 
      resultSet[1].buffer_type = MYSQL_TYPE_LONG;
      resultSet[1].buffer = (void *)&MAX;
      resultSet[1].is_null = 0;
      resultSet[1].is_unsigned = 1;
      // 'LPARAM' 
      resultSet[2].buffer_type = MYSQL_TYPE_LONG;
      resultSet[2].buffer = (void *)&LPARAM;
      resultSet[2].is_null = 0;
      resultSet[2].is_unsigned = 1;
      // 'NRETRY' 
      resultSet[3].buffer_type = MYSQL_TYPE_LONG;
      resultSet[3].buffer = (void *)&NRETRY;
      resultSet[3].is_null = 0;
      resultSet[3].is_unsigned = 1;
      if (res = sqlBindResult (stmt, resultSet))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	if (res = sqlExecStmt (stmt))
	  syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
	else 
	  mysql_stmt_fetch (stmt);
      if (sqlCloseStmt (&stmt))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
    }
  return res;
}

int
initWindLevels (MYSQL *conn)
{
  MYSQL_STMT *stmt;
  MYSQL_BIND resultSet[11];
  // SELECT_ENABLE_LEVELS output buffers
  int id, 
      vp,
      vmax,
      vmin,
      iv,
      sup, 
      low,
      hys,
      evt,
      ntv,
      smp,
      res = 0;
  stateLst rows;

  if (res = sqlPrepareStmt (conn, &stmt, SELECT_ENABLE_LEVELS))
    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else
    {
      // Set output buffers for windLevels resultset
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
      // 'int'
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
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	{	
	  if (res = sqlExecStmt (stmt))
	    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
	  else 
	    {
	      // initialize state descriptors table
	      rows = mysql_stmt_num_rows (stmt);
	      while (!mysql_stmt_fetch (stmt))
		if (!add2StateDscTbl (id, id, vp, sup/DPERIOD, low/DPERIOD, evt, ntv, smp))
		  syslog (LOG_ERR, "error in %s (file %s at line %d): cannot add state descriptor at position %d", 
			  __func__, __FILE__, __LINE__, id); 
	    }
	}
      if (sqlCloseStmt (&stmt))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
    }
  return res;
}

int 
setPlcState (MYSQL *conn, const int unsigned id, const unsigned int status)
{
  int res;
  MYSQL_STMT *stmt;
  MYSQL_BIND paramU[2];

  if (res = sqlPrepareStmt (conn, &stmt, SET_PLC_SATUS))
    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else 
    {
      memset (paramU, 0, sizeof (paramU));
      // 'sts'
      paramU[0].buffer_type = MYSQL_TYPE_TINY;
      paramU[0].buffer = (void *)&status;
      paramU[0].is_null = 0;
      paramU[0].is_unsigned = 0;
      // 'id'
      paramU[1].buffer_type = MYSQL_TYPE_TINY;
      paramU[1].buffer = (void *)&id;
      paramU[1].is_null = 0;
      paramU[1].is_unsigned = 0;
      if (res = sqlBindParam (stmt, paramU))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else 
	if (res = sqlExecStmt (stmt))
	  syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      if (sqlCloseStmt (&stmt))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
    }
  return res;
}

int 
setLiveDta (MYSQL *conn, const int unsigned id, const bool enable)
{
  int res;
  char *live;
  MYSQL_STMT *stmt;
  MYSQL_BIND paramI[1];

  switch (enable)
    {
    case true:
      live = INIT_LIVE_DTA;
      break;
    default:
      live = CLEAR_LIVE_DTA;
      break;
    }
  if (res = sqlPrepareStmt (conn, &stmt, live))
    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else 
    {
      // 'id'
      memset (paramI, 0, sizeof (paramI));
      paramI[0].buffer_type = MYSQL_TYPE_TINY;
      paramI[0].buffer = (void *)&id;
      paramI[0].is_null = 0;
      paramI[0].is_unsigned = 0;
      if (res = sqlBindParam (stmt, paramI))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else 
	if (res = sqlExecStmt (stmt))
	  syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
      if (sqlCloseStmt (&stmt))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_stmt_error(stmt));
    }
  return res;
}

int 
setBackLog (MYSQL *conn, const bool enable)
{
  MYSQL_STMT *stmt;
  int res = 0;
  char *log;

  switch (enable)
    {
    case true:
      log = ENABLE_LOG;
      break;
    default:
      log = DISABLE_LOG;
      break;
    }
  if (res = mysql_query (conn, log))
      syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
  return res;
}

void *
doWork (void *argv)
{
  MYSQL *conn;
  MYSQL_STMT *stmt;
  MYSQL_BIND paramU[6];
  bool exit = false;
  int *value;
  int i,
      res,
      vp,
      level,
      disconnect = 1,
      // signal received in sigwait (SIGRTMIN based or SIGTERM)
      rsig,
      aperiod = APERIOD,
      plcErr = 0,
      sqlErr = 0;
  periodDscPtr pd;
  daveConnection *dc;
  //daveInterface *di;
  pDataPtr dta;

  dta = (pDataPtr) argv;
  // get wind level parameters
  switch (LPARAM)
    {
    case 1:
      value = &vp;
      break;
    case 2:
      value = &(dta->max);
      break;
    case 3:
      value = &(dta->min);
      break;
    case 4:
      value = &(dta->avg);
      break;
    default:
      value = &vp;
      break;
    }
  // connect to mysql data store
  if (res = sqlConnect (&conn, 1))
    {
      syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", __func__, dta->id, dta->ip, __FILE__, __LINE__, mysql_error(conn));
      pthread_exit (NULL);
    }
  if (res = sqlPrepareStmt (conn, &stmt, UPDATE_LIVE_DTA))
    syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", __func__, dta->id,  dta->ip, __FILE__, __LINE__, mysql_stmt_error(stmt));
  else
    {
      // Set output buffers for windLiveLevels
      memset (paramU, 0, sizeof (paramU));
      // 'vp'
      paramU[0].buffer_type = MYSQL_TYPE_TINY;
      paramU[0].buffer = (void *)&vp;
      paramU[0].is_null = 0;
      paramU[0].is_unsigned = 1;
      // 'vmax'
      paramU[1].buffer_type = MYSQL_TYPE_TINY;
      paramU[1].buffer = (void *)&(dta->max);
      paramU[1].is_null = 0;
      paramU[1].is_unsigned = 1;
      // 'vmin'
      paramU[2].buffer_type = MYSQL_TYPE_TINY;
      paramU[2].buffer = (void *)&(dta->min);
      paramU[2].is_null = 0;
      paramU[2].is_unsigned = 1;
      // 'iv'
      paramU[3].buffer_type = MYSQL_TYPE_TINY;
      paramU[3].buffer = (void *)&(dta->avg);
      paramU[3].is_null = 0;
      paramU[3].is_unsigned = 1;
      // 'lvl'
      paramU[4].buffer_type = MYSQL_TYPE_TINY;
      paramU[4].buffer = (void *)&(level);
      paramU[4].is_null = 0;
      paramU[4].is_unsigned = 0;      
      // 'id' 
      paramU[5].buffer_type = MYSQL_TYPE_TINY;
      paramU[5].buffer = (void *)&(dta->id);
      paramU[5].is_null = 0;
      paramU[5].is_unsigned = 0;
      if (res = sqlBindParam (stmt, paramU))
	syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", __func__, dta->id, dta->ip, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	{
	  // make thread periodic
	  if (pd = make_periodic (DPERIOD, dta->sig))
	    {
	      // infinte loop 
	      while (!exit)
		{
		  if (disconnect) 
		    {
#ifdef DEBUG
		      syslog (LOG_INFO, "connecting to plc %d: %s...", dta->id, dta->ip);
#endif
	              disconnect = plcConnect (dta->ip, &dc);
		    }
		  // if disconnected from plc...
		  if (disconnect)
		    {
		      plcErr++;
		      syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s",
			      __func__, dta->id, dta->ip, __FILE__, __LINE__, daveStrerror(disconnect));
		    }
		  // successfull connected...
		  else 
		    {
		      res = daveReadBytes (dc, daveDB, DAREA, 0, DLEN + ALEN, NULL);
		      if (res)
			// ...error reading...
			{
			  // ...increment error count...
			  plcErr++;
			  syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", 
				  __func__, dta->id, dta->ip, __FILE__, __LINE__, daveStrerror(res));
			}
		      // ...successfull read from plc...
		      else
			{
			  // ...reset error count...
			  plcErr = 0;
			  // ...convert data for storing into mysql data store...
			  // Anamemoter 1
			  dta->vp1 = daveGetU16At (dc, 2);
			  // Anamemoter 2
			  dta->vp2 = daveGetU16At (dc, 4);
			  // Spreader's weight
			  dta->wgt = daveGetU16At (dc, 6);
			  // Spreader's height
			  dta->hgt = daveGetS16At (dc, 8);
			  // driver's cab position
			  dta->pos = daveGetS16At (dc, 10);
			  // infos
			  for (i = 0; i < sizeof (dta->nfo); i++)
			    {
			      dta->nfo[i] = daveGetU8At (dc, 14 + (i * sizeof (unsigned char)));
#ifdef DEBUG
			      syslog (LOG_INFO, "plc %s: nfo[%d] %u", dta->ip, i, dta->nfo[i]);
#endif
			    }
			  // alarms
			  for (i = 0; i < sizeof (dta->alm); i++)
			    {
			      dta->alm[i] = daveGetU8At (dc, 18 + (i * sizeof (unsigned char)));
#ifdef DEBUG
			      syslog (LOG_INFO, "plc %s: alm[%d] %u", dta->ip, i, dta->alm[i]);
#endif
			    }

//			  vp = 1 + (int) (40.0 * (rand() / (RAND_MAX + 1.0)));

			  // get wind data from anemometer 1 if not faulty...
			  // ...otherwise, get data from anemometr 2 if not faulty...
			  vp = (dta->vp1 < MAXWIND) ? dta->vp1 : ((dta->vp2 < MAXWIND) ? dta->vp2 : -1);
			  // ...otherwise exit! 
			  if (vp < 0)
			    {
			      syslog (LOG_CRIT, "error %s of plc %d: %s (file %s at line %d): both anemometer are in error",
				      __func__, dta->id, dta->ip, __FILE__, __LINE__);
			      exit = true;
			    }
			  // ...calculate min, max and avg (moving average)...
			  processData (dta, vp);
			  // ...get anemometer new state based on value...
			  level = dta->act->state->newState (dta->act, *value);
#ifdef DEBUG
			  syslog (LOG_INFO, "plc %s: vp  %d", dta->ip, vp);
			  syslog (LOG_INFO, "plc %s: vp1 %d", dta->ip, dta->vp1);
			  syslog (LOG_INFO, "plc %s: vp2 %d", dta->ip, dta->vp2);
			  syslog (LOG_INFO, "plc %s: wgt %d", dta->ip, dta->wgt);
			  syslog (LOG_INFO, "plc %s: hgt %d", dta->ip, dta->hgt);
			  syslog (LOG_INFO, "plc %s: pos %d", dta->ip, dta->pos);
			  syslog (LOG_INFO, "plc %s: level(%d), up counter(%d), down counter(%d)", 
				  dta->ip, dta->act->state->level, dta->act->ucount, dta->act->dcount);
//			  syslog (LOG_INFO, "plc %s: alarm(%d), down counter(%d)", 
//				  dta->ip, dta->act->alarm, dta->act->alevel);
#endif
			  // ...update mysql data store...
			  if (res = sqlExecStmt (stmt))
			    // ...error updating...
			    {
			      syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", 
				      __func__, dta->id, dta->ip, __FILE__, __LINE__, mysql_stmt_error(stmt));
			      // ...if connection error to mysql, exit!
			      if (res == CR_SERVER_GONE_ERROR || res == CR_SERVER_LOST || CR_SERVER_LOST_EXTENDED)
				exit = true;
			    }
			}
		      // disconnect from plc (at the end of each cycle)
		      if (!plcDisconnect (dc))
			{
#ifdef DEBUG
			  syslog (LOG_INFO, "disconnecting to plc %d: %s...", dta->id, dta->ip);
#endif
			  disconnect = 1;
			}
		    }
		  // wait next period or SIGTERM anyway!
		  if (!wait_period (pd, &rsig))
		    {
		    // if SIGTERM received, exit
		    if (rsig == SIGTERM)
		      exit = true;
		    }
		  // if wait_period fails logg it
		  else 
		    syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): error waiting for next period", 
			    __func__, dta->id, dta->ip, __FILE__, __LINE__);
		  // if there were NRETRY consecutive error, exits!
		  if (plcErr >= NRETRY)
		    exit = true;
		}
	      // delete periodic timer
	      if (res = timer_delete (pd->timerId))
		syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s",
			__func__, dta->id, dta->ip, __FILE__, __LINE__, strerror(res));
	      // free periodic descriptor
	      free (pd);
	    }
	}
    }
  if (sqlCloseStmt (&stmt))
    {
      syslog (LOG_CRIT, "error in %s of plc %d: %s (file %s at line %d): %s", 
	      __func__, dta->id, dta->ip, __FILE__, __LINE__, mysql_stmt_error(stmt));
      pthread_exit (NULL);
    }
  // disconnect from mysql data store
  sqlDisconnect (&conn);
}

int
main (int argc, char *argv[])
{
  int c, 
      rsig, 
      i = 0, 
      res = 0,
      died = 0,
      NTHS;
  char *cvalue = NULL, 
       *pidname = NULL, 
       *pidext = NULL;
  MYSQL *conn;
  pDataPtr *plcsDta = NULL;
  periodDscPtr pd = NULL;

  // Parse startup parameters
  openlog (pidname, LOG_PID, LOG_LOCAL5);
  opterr = 0;
  while ((c = getopt (argc, argv, "p:")) != -1)
    switch (c)
      {
      case 'p':
	cvalue = optarg;
	break;
      case '?':
	if (optopt == 'p')
	  syslog (LOG_ERR, "option -%c requires an argument, code=%d (%s)",
		  optopt, errno, strerror (errno));
	else if (optopt != 0)
	  syslog (LOG_ERR, "unknown option '-%c', code=%d (%s)",
		  optopt, errno, strerror (errno));
	break;
      default:
	break;
      }
  if (cvalue == NULL)
    {
      syslog (LOG_ERR, "exit due to invalid startup options");
      exit (EXIT_FAILURE);
    }
  // initialize the logging interface
  pidfile = (char *) malloc ((strlen (cvalue) + 1) * sizeof (char));
  pidfile = strcpy (pidfile, cvalue);
  pidname = strtok (basename (cvalue), ".");
  pidext = strtok (NULL, ".");

  // connect to mysql data store
  if (res = sqlConnect (&conn, 1))
    {
      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
      mysql_library_end ();
      exit (EXIT_FAILURE);
    }
  // init PLC's configuration parameters
  if (res = initConfParams (conn))
    {
      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
      sqlDisconnect (&conn);
      mysql_library_end ();
      exit (EXIT_FAILURE);
    }
  // init PLC's state descriptors table
  if (res = initWindLevels (conn))
    {
      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
      sqlDisconnect (&conn);
      mysql_library_end ();
      exit (EXIT_FAILURE);
    }
  // get working PLC's data
  if (!(plcsDta = initPlcsData (conn, &NTHS)))
    {
      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
      sqlDisconnect (&conn);
      mysql_library_end ();
      exit (EXIT_FAILURE);
    }
  // enable backlog of windLiveValues
  if (res = setBackLog (conn, true))
    {
      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
      sqlDisconnect (&conn);
      mysql_library_end ();
      exit (EXIT_FAILURE);
    }

  // ATTENTION!!! MAKE OURSELF A DAEMON !!!
  daemonize (pidfile);
  // NOW WE'RE IN HELL...DO BAD THINGS !!!

  // spawn PLC's threads
  pthread_t plc[NTHS];
  for (i = 0; i < NTHS; i++)
    {
      if (res = pthread_create (&plc[i], NULL, doWork, (void *)plcsDta[i]))
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, strerror (errno));
      else
	// set thread status to 'RUN'
	res = setPlcState (conn, plcsDta[i]->id, RUN);
    }
  // waiting for termination while periodically check for dead threads
  pd = make_periodic (60, SIGUSR1);
  while (pd)
    if (!wait_period (pd, &rsig))
      {
	// SIGTERM received, exits
	if (rsig == SIGTERM)
	  {
	    free (pd);
	    pd = NULL;
	  }
	else
	// SIGUSR1 received (timer expiration) check for dead threads
	  {
	    for (i = 0; i < NTHS; i++)
	      // if thread could not be found, log it & count it
	      if (pthread_kill (plc[i], 0) == ESRCH)
		{
		  syslog (LOG_ERR, "error in %s (file %s at line %d): thread %s, %s", __func__, __FILE__, __LINE__, plcsDta[i]->ip, strerror (ESRCH));
		  died ++;
		  // check if connection to mysql data store is still up
		  if (!mysql_ping (conn))
		    // set thread status to 'ERR'
		    res = setPlcState (conn, plcsDta[i]->id, ERR);
		  else
		    syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error (conn));
		}
	    // all threads died, exits
	    if (died == NTHS)
	      {
		free (pd);
		pd = NULL;
	      }
	  }
      }
  // if there're threads still alive...
  if (died < NTHS) 
    {
      // cancel and join all PLC's threads still living
      for (i = 0; i < NTHS; i++)
	if (!pthread_kill (plc[i], SIGTERM))
	  if (!pthread_join (plc[i], NULL))
	    // check if connection to mysql data store is still up
	    if (!mysql_ping (conn))
	      // set thread status to 'STP'
	      res = setPlcState (conn, plcsDta[i]->id, STP);
	    else
	      syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error (conn));
      // check if connection to mysql data store is still up
      if (!mysql_ping (conn))
	{
	  // disable backlog of windLiveValues
	  if (res = setBackLog (conn, false))
	    {
	      syslog (LOG_EMERG, "exit due to critical error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error(conn));
	      sqlDisconnect (&conn);
	      mysql_library_end ();
	      exit (EXIT_FAILURE);
	    }
	}
      else
	syslog (LOG_ERR, "error in %s (file %s at line %d): %s", __func__, __FILE__, __LINE__, mysql_error (conn));
    }
  // ...else log that we exited due to critical error
  else
    syslog (LOG_EMERG, "error in %s (file %s at line %d): all threads died unexpectedly", __func__, __FILE__, __LINE__);
  // disconnect from mysql data store
  sqlDisconnect (&conn);
  mysql_library_end ();
  // clear working PLC's data
  freePlcsData (plcsDta, NTHS);
  // reset PLC's state descriptors table
  resetStateDscTbl ();
  syslog (LOG_INFO, "exit gracefully");
  return res;
}
