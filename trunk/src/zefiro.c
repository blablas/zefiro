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

void
daemonize (const char *pidfile)
{
  pid_t pid, sid, parent;
  //struct sigaction saio;
  sigset_t waitMask, exitMask;
  int rsig;
  char cpid[5] = { 0x00 };

  // block all signals
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
  sigemptyset (&exitMask);
  sigaddset (&exitMask, SIGCHLD);
  sigaddset (&exitMask, SIGTERM);
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
      sigwait (&exitMask, &rsig);
      exit (EXIT_FAILURE);
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

  /* 
     Change the current working directory.  This prevents the current
     directory from being locked; hence not being able to remove it. 
   */
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

pDataPtr*
initPlcsData (MYSQL *conn, int *len)
{
  pDataPtr *dta = NULL;
  MYSQL_STMT *stmt;
  MYSQL_BIND resultSet[2];
  // SELECT_PLC_DTA output buffers
  char ip[15];
  int id, 
      res = 0,
      pos = 0;

  if (res = sqlPrepareStmt (conn, &stmt, SELECT_PLC_DTA))
    syslog (LOG_ERR, "sqlPrepareStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
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
	syslog (LOG_ERR, "sqlBindResult (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	{	
	  if (res = sqlExecStmt (stmt))
	    syslog (LOG_ERR, "sqlExecStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
	  else 
	    {
	      // initialize plc data (id, ip)
	      *len = mysql_stmt_num_rows (stmt);
	      if (dta = malloc (sizeof (struct pData) * (*len)))
		while (!mysql_stmt_fetch (stmt))
		  {
		    dta[pos]->id = id;
		    strcpy (dta[pos]->ip, ip);
		    pos++;
		  }
	      else
		syslog (LOG_ERR, "initPlcsData in file %s at line %d: cannot add state descriptor at position %d",
			__FILE__, __LINE__ ,pos);
	    }
	}
    }
  if (sqlCloseStmt (&stmt))
    syslog (LOG_ERR, "sqlCloseStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
  return dta;
}

int
initWindLevels (MYSQL *conn)
{
  MYSQL_STMT *stmt;
  MYSQL_BIND resultSet[8];
  // SELECT_ENABLE_LEVELS output buffers
  int id, 
      vp,
      vmax,
      vmin,
      iv,
      sup, 
      low,
      hys,
      res = 0;
  stateLst pos = A, rows;

  if (res = sqlPrepareStmt (conn, &stmt, SELECT_ENABLE_LEVELS))
    syslog (LOG_ERR, "sqlPrepareStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
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
      if (res = sqlBindResult (stmt, resultSet))
	syslog (LOG_ERR, "sqlBindResult (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
      else
	{	
	  if (res = sqlExecStmt (stmt))
	    syslog (LOG_ERR, "sqlExecStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
	  else 
	    {
	      // initialize state descriptors table
	      rows = mysql_stmt_num_rows (stmt);
	      while (!mysql_stmt_fetch (stmt))
		{
		  if (pos == (rows - 1))
		    pos = E;
		  if (!add2StateDscTbl (pos, id, vp, sup, low, runStateTbl[pos]))
		    syslog (LOG_ERR, 
			    "add2StateDscTbl in file %s at line %d: cannot add state descriptor at position %d", 
			    __FILE__, __LINE__, pos); 
		  pos++;
		}
	    }
	}
    }
  if (sqlCloseStmt (&stmt))
    syslog (LOG_ERR, "sqlCloseStmt (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
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
      syslog (LOG_ERR, "mysql_query (%d) in file %s at line %d: %s", res, __FILE__, __LINE__, mysql_stmt_error(stmt));
  return res;
}

int
main (int argc, char *argv[])
{
  sigset_t exitMask;
  int c, i = 0, rsig, res = 0;
  unsigned int len = 0;
  char *cvalue = NULL, *pidname = NULL, *pidext = NULL;

  MYSQL *conn;
  pDataPtr *plcsDta = NULL;

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
      syslog (LOG_ERR, "sqlConnect (%d) in file %s at line %d", res, __FILE__, __LINE__);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  // get working PLC's data
  if (!(plcsDta = initPlcsData (conn, &len)))
    {
      sqlDisconnect (&conn);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  for (i = 0; i < len; i++)
    {
      syslog (LOG_NOTICE, "plcsDta length %d", len);
      syslog (LOG_NOTICE, "plcsDta[%d]->id %d", i, plcsDta[i]->id);
      syslog (LOG_NOTICE, "plcsDta[%d]->ip %s", i, plcsDta[i]->ip);
    }
  free (plcsDta);
  // init PLC's state descriptors table
  if (res = initWindLevels (conn))
    {
      sqlDisconnect (&conn);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  for (i = 0; i < 5; i++)
    if (stateDscTbl[i])
      {
	syslog (LOG_NOTICE, "stateDscTbl[%d]->label %d", i, stateDscTbl[i]->label);
	syslog (LOG_NOTICE, "stateDscTbl[%d]->il %d", i, stateDscTbl[i]->il);
	syslog (LOG_NOTICE, "stateDscTbl[%d]->sup %d", i, stateDscTbl[i]->sup);
	syslog (LOG_NOTICE, "stateDscTbl[%d]->low %d", i, stateDscTbl[i]->low);
	syslog (LOG_NOTICE, "stateDscTbl[%d]->newState %d", i, stateDscTbl[i]->newState);
      }
  // enable backlog of windLiveValues
  if (res = setBackLog (conn, true))
    {
      sqlDisconnect (&conn);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  syslog (LOG_NOTICE, "backlog enabled");
  // disconnect from mysql data store
  sqlDisconnect (&conn);

  // Daemonize
  daemonize (pidfile);

  // Now we are a daemon -- do the followings:

  // wait for termination 
  sigemptyset (&exitMask);
  sigaddset (&exitMask, SIGTERM);
  syslog (LOG_NOTICE, "waiting for termination signal in file %s at line %d", __FILE__, __LINE__);  
  sigwait (&exitMask, &rsig);
  syslog (LOG_NOTICE, "termination signal %d received in file %s at line %d", rsig, __FILE__, __LINE__);  
  // reset PLC's state descriptors table
  resetStateDscTbl ();
  syslog (LOG_NOTICE, "clear state descriptor table in file %s at line %d",  __FILE__, __LINE__);

  // connect to mysql data store
  if (res = sqlConnect (&conn, 1))
    {
      syslog (LOG_ERR, "sqlConnect (%d) in file %s at line %d", res, __FILE__, __LINE__);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  // disable backlog of windLiveValues
  if (res = setBackLog (conn, false))
    {
      sqlDisconnect (&conn);
      mysql_library_end();
      exit (EXIT_FAILURE);
    }
  syslog (LOG_NOTICE, "backlog disabled");
  return 0;
}
