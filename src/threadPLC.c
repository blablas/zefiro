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
#include "threadPLC.h"

periodDscPtr
make_periodic (const int unsigned period, const int unsigned sig)
{
  periodDscPtr pDsc = NULL;
  //timer_t timerId;
  struct sigevent sigev;
  struct itimerspec itval;

  pDsc = malloc (sizeof (struct periodDsc));
  if (pDsc)
    {
      pDsc->sig = sig;
      // create the signal mask that will be used in wait_period
      sigemptyset (&(pDsc->timerMask));
      sigaddset (&(pDsc->timerMask), pDsc->sig);
      sigaddset (&(pDsc->timerMask), SIGTERM);
      // Create a timer that will generate the signal we have chosen
      sigev.sigev_notify = SIGEV_SIGNAL;
      sigev.sigev_signo = pDsc->sig;
      sigev.sigev_value.sival_ptr = (void *) &(pDsc->timerId);
      if (timer_create (CLOCK_MONOTONIC, &sigev, &(pDsc->timerId)))
	{
	  free (pDsc);
	  return NULL;
	}
      // make the timer periodic
      itval.it_interval.tv_sec = period;
      itval.it_interval.tv_nsec = 0;
      itval.it_value.tv_sec = period;
      itval.it_value.tv_nsec = 0;
      if (timer_settime (pDsc->timerId, 0, &itval, NULL))
	{
	  free (pDsc);
	  return NULL;
	}
    }
  return pDsc;
}

int 
wait_period (periodDscPtr pDsc, int *rsig)
{
  return sigwait (&(pDsc->timerMask), rsig);
}
