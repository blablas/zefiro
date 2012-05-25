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
#include "statePLC.h"

// PLC's state functions table
const runState runStateTbl[] = {
    runStateA, 
    runStateBCD,
    runStateBCD,
    runStateBCD,
    runStateE
};

// PLC's state table
stateDscPtr stateDscTbl[] = { NULL, NULL, NULL, NULL, NULL };

// find PLC's next and previous state 
inline stateDscPtr
nextState (stateLst state)
{
  int i;
  
  for (i = state; i < E; i++)
    if (stateDscTbl[i + 1])
      return stateDscTbl[i + 1];
}

inline stateDscPtr
prevState (stateLst state)
{
  int i;
  
  for (i = state; i > A; i--)
    if (stateDscTbl[i - 1])
      return stateDscTbl[i - 1];
}

#ifndef SAMPLING
inline void
upgradeState (actualStatePtr actual, stateDscPtr tmp)
{
  // reset downgrade counter to low threshold
  actual->dcount = tmp->low;
  // if upgrade counter fires...
  if (!(--(actual->ucount)))
    {
      // upgrade actual state...
      actual->state = nextState (tmp->level);
      // ...initialize actual state thresholds...
      actual->ucount = (actual->state)->sup;
      actual->dcount = (actual->state)->low;
    }
}

inline void 
downgradeState (actualStatePtr actual, stateDscPtr tmp)
{
  // reset upgrade counter to sup threshold
  actual->ucount = tmp->sup;
  // if downgrade counter fires...
  if (!(--(actual->dcount)))
    {
       // downgrade actual state...
       actual->state = prevState (tmp->level);
       // ...and set corrisponding thresholds
       actual->dcount = (actual->state)->low;	      
       actual->ucount = (actual->state)->sup;	      
    }
}

inline void
resetState (actualStatePtr actual, stateDscPtr tmp)
{
  // reset upgrade counter to sup threshold
  actual->ucount = tmp->sup;	  
  // reset downgrade counter to low threshold
  actual->dcount = tmp->low;	  
}
#endif
#ifdef SAMPLING
inline void
resetState (actualStatePtr actual, stateDscPtr tmp)
{
  // manage reset of upgrade stuff (only if we're actually processing state upgrade!)
  if (actual->processState)
    {
      // ...reset interval counter (cause an event does not occurred)...
      actual->icount = tmp->interval;
      // ...reset events counter (cause an event does not occurred)...
      actual->ecount = tmp->events;
      // ...reset samples counter (cause an event does not occurred)...
      actual->scount = tmp->samples;
    }
  // if we're counting interval...
  else
    // ...if interval counter fires...
    if (!(--(actual->icount)))
      {
        // ...pass in processing state..
        actual->processState = true;
	// ...and reload interval counter
	actual->icount = tmp->interval;
      }
  // reset downgrade counter to low threshold
  actual->dcount = tmp->low;	  
}

inline void
upgradeState (actualStatePtr actual, stateDscPtr tmp)
{
  // reset downgrade counter to low threshold
  actual->dcount = tmp->low;
  // manage state upgrade
  // if not in process state...
  if (!(actual->processState))
    {
      // ...if interval counter fires...
      if (!(--(actual->icount)))
	{
          // ...pass in processing state..
	  actual->processState = true;
	  // ...and reload interval counter
	  actual->icount = tmp->interval;
	}
    }
  // ...if we're processing state...
  else
    {
      // if samples counter fires...
      if (!(--(actual->scount)))
	{
	  // ...and if events counter fires...
	  if (!(--(actual->ecount)))
	    {
	      // upgrade actual state...
	      actual->state = nextState (tmp->level);
	      // ...initialize actual state thresholds...
	      actual->dcount = (actual->state)->low;
	      actual->icount = (actual->state)->interval;
	      actual->scount = (actual->state)->samples;
	      actual->ecount = (actual->state)->events;
	    }  
	  // ...otherwise, if events counter doesn't fires (we still had an event!)...
	  else
	    {
	      // ...reset interval counter (cause an event occurred)... 
	      actual->icount = tmp->interval;
	      // ...reset samples counter (cause an event occurred)...
	      actual->scount = tmp->samples;
	      // ...exit process state (cause an event occurred)...
	      actual->processState = false;
	    }
	}
      // ...otherwise we just had a upper sample!
    }
}

inline void 
downgradeState (actualStatePtr actual, stateDscPtr tmp)
{
  // if downgrade counter fires...
  if (!(--(actual->dcount)))
    {
       // downgrade actual state...
       actual->state = prevState (tmp->level);
       // ...initialize actual state thresholds...
       actual->dcount = (actual->state)->low;
       actual->icount = (actual->state)->interval;
       actual->scount = (actual->state)->samples;
       actual->ecount = (actual->state)->events;
       // ...enter (or stay in) process state
       actual->processState = true;
    }
  else
    {
      if (actual->processState)
	{
	  // ...reset events counter (cause an event does not occurred)...
	  actual->ecount = tmp->events;
	  // ...reset samples counter (cause an event does not occurred)...
	  actual->scount = tmp->samples;
	}
      else
	if (!(--(actual->icount)))
	  {
	    actual->processState = true;
            actual->icount = tmp->interval;
	  }
    }
}
#endif 
actualStatePtr 
initActualState (const stateLst state)
{
  actualStatePtr asPtr = NULL;
 
  if (stateDscTbl[state] != NULL)
    {
      asPtr = malloc (sizeof (struct actualState));
      if (asPtr)
	{
	  asPtr->state = stateDscTbl[state];
	  asPtr->ucount = (asPtr->state)->sup;
	  asPtr->dcount = (asPtr->state)->low;
#ifdef SAMPLING
	  asPtr->ecount = (asPtr->state)->events;
	  asPtr->icount = (asPtr->state)->interval;
	  asPtr->scount = (asPtr->state)->samples;
	  asPtr->processState = true;
#endif
	}
    }
  return asPtr;
}

void disposeActualState (actualStatePtr actual)
{
  if (actual)
    free (actual);
}

stateDscPtr
add2StateDscTbl (const stateLst pos, const stateLst level, const int il, const int sup, const int low, 
		 const int evt, const int ntv, const int smp)
{
  stateDscPtr dsPtr = NULL;
 
  dsPtr = malloc (sizeof (struct stateDsc));
  if (dsPtr)
    {
      dsPtr->level = level;
      dsPtr->il = il;
      dsPtr->sup = sup;
      dsPtr->low = low;
#ifdef SAMPLING
      dsPtr->events = evt;
      dsPtr->interval = ntv;
      dsPtr->samples = smp;
#endif
      dsPtr->newState = runStateTbl[level];
      if (stateDscTbl[pos] != NULL)
	free (stateDscTbl[pos]);
      stateDscTbl[pos] = dsPtr;
    }
  return dsPtr;
}

void 
resetStateDscTbl (void)
{
  stateLst i;

  for (i = A; i < Z; i++)
    if (stateDscTbl[i] != NULL)
      {
	free (stateDscTbl[i]);
	stateDscTbl[i] = NULL;
      }
}

stateLst  
runStateA (actualStatePtr actual, const int value)
{  
  stateDscPtr tmp = actual->state;

  // value > istant threshold
  if (value > tmp->il)
    upgradeState (actual, tmp);
  // value <= instant threshold
  else 
    resetState (actual, tmp);
  return (actual->state)->level;
}

stateLst  
runStateBCD (actualStatePtr actual, const int value)
{
  stateDscPtr tmp = actual->state;

  // value > istant threshold
  if (value > tmp->il)
    upgradeState (actual, tmp);
  else 
    {
      // value <= previuos instant threshold
      if (value <= (prevState (tmp->level))->il)
	downgradeState (actual, tmp);
      // previous instant threshold < value <= instant threshold
      else 
	resetState (actual, tmp);
    }
  return (actual->state)->level;
}

stateLst  
runStateDummy (actualStatePtr actual, const int value)
{
  return (actual->state)->level;
}

stateLst  
runStateE (actualStatePtr actual, const int value)
{
  stateDscPtr tmp = actual->state;

  // value < istant threshold
  if (value < tmp->il)
    downgradeState (actual, tmp);
  // reset downgrade counter to low threshold
  else 
    resetState (actual, tmp);
  return (actual->state)->level;
}

