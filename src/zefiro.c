/*
** Copyright (C) 2009 Emiliano Giovannetti <emiliano.giovannetti@tdt.it>
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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

int lfp = -1;
char *pidfile = NULL;

static void
child_handler (int signum)
{
  switch (signum)
    {
    case SIGTERM:
      //dbdisconnect ();
      exit (EXIT_SUCCESS);
      break;
    case SIGCHLD:
      exit (EXIT_SUCCESS);
      break;
    case SIGALRM:
      exit (EXIT_SUCCESS);
      break;
    default:
      //dbdisconnect ();
      exit (EXIT_FAILURE);
      break;
    }
}

static void
daemonize (const char *pidfile)
{
  pid_t pid, sid, parent;
  struct sigaction saio;
  char cpid[5] = { 0x00 };

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

  // Install the signal handler before making the device asynchronous 
  saio.sa_handler = child_handler;
  sigfillset (&saio.sa_mask);	// mask all signal during handler execution 
  saio.sa_flags = 0;
  saio.sa_restorer = NULL;
  sigaction (SIGCHLD, &saio, NULL);
  sigaction (SIGTERM, &saio, NULL);
  sigaction (SIGALRM, &saio, NULL);

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
      wait (NULL);
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

  kill (parent, SIGALRM);

  // Drop user if there is one, and we were run as root 
  if (getuid () == 0 || geteuid () == 0)
    {
      struct passwd *pw = getpwnam (RUN_AS_USER);
      if (pw)
	setuid (pw->pw_uid);
    }
}

int
main (int argc, char *argv[])
{
  int c, i,j;

  long result = 0;

  char *cvalue = NULL, *pidname = NULL, *pidext = NULL;

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

  // Initialize the logging interface
  pidfile = (char *) malloc ((strlen (cvalue) + 1) * sizeof (char));
  pidfile = strcpy (pidfile, cvalue);
  pidname = strtok (basename (cvalue), ".");
  pidext = strtok (NULL, ".");

  // Daemonize
  daemonize (pidfile);

  /* 
     Now we are a daemon -- do the followings:
   */

  while (1)
    {

    }				// end while(1)

  // Should never reach this point
  return 0;
}
