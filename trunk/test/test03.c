#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <errno.h>
#include "threadPLC.h"

#define NTHREAD		2

void *tFunc (void *argv)
{
  int rsig;
  periodDscPtr pd;

  pd = make_periodic (1, *((int *)argv));
  while (pd)
    {
      printf ("thread %d, go to sleep waiting for signal %d\n", pthread_self(), *((int *)argv));
      wait_period (pd, &rsig);
      printf ("thread %d signal received: %d\n", pthread_self(), rsig);
    }
  free (pd);
}

int
main (void)
{
  pthread_t plc[NTHREAD];
  sigset_t timerMask, waitMask;
  //struct sigaction saio;
  int i, sig[NTHREAD], rsig;

  // mask SIGRT signals for all threads
  sigemptyset (&timerMask);
  for (i = 0; i < NTHREAD; i++)
    sigaddset (&timerMask, SIGRTMIN + i); 
  
  // mask SIGCHLD signal for main process termination
  sigaddset (&timerMask, SIGCHLD); 
  sigprocmask (SIG_BLOCK, &timerMask, NULL);

  for (i = 0; i < NTHREAD; i++)
    {
      sig[i] = SIGRTMIN + i;
      if (pthread_create (&plc[i], NULL, tFunc, (void *)&sig[i]))
	break;
      else ("thread %d created\n", plc[i]);
    }

  sigemptyset (&waitMask);
  sigaddset (&waitMask, SIGCHLD); 
  sigwait (&waitMask, &rsig);

  // cancel and join all child threads
  for (i = 0; i < NTHREAD; i++)
    {
      pthread_cancel (plc[i]);
      pthread_join (plc[i], NULL);
      printf ("thread %d cancelled\n", plc[i]);
    }

  return 0;
}

