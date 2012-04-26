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
#ifndef THREADPLC_H
#define THREADPLC_H
#include <pthread.h>
#include <signal.h>
#include <time.h>

typedef struct periodDsc * periodDscPtr;

// PLC's periodc API
extern periodDscPtr make_periodic (const int unsigned period, const int unsigned sig);
extern int wait_period (periodDscPtr pDsc, int *rsig);

struct periodDsc
{
  int sig;
  timer_t timerId;
  sigset_t timerMask;
};
#endif
