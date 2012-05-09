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
  if (state < E)
    do
      state++;
    while (!stateDscTbl[state]);
  return stateDscTbl[state];
}

inline stateDscPtr
prevState (stateLst state)
{
  if (state > A)
    do
      state--;
    while (!stateDscTbl[state]);
  return stateDscTbl[state];
}

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
      	  asPtr->alarm = state;
	  asPtr->ucount = (asPtr->state)->sup;
	  asPtr->dcount = (asPtr->state)->low;
	  asPtr->alevel = asPtr->dcount;
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
add2StateDscTbl (const stateLst pos, const stateLst level, const int il, const int sup, const int low)
{
  stateDscPtr dsPtr = NULL;
 
  dsPtr = malloc (sizeof (struct stateDsc));
  if (dsPtr)
    {
      dsPtr->level = level;
      dsPtr->il = il;
      dsPtr->sup = sup;
      dsPtr->low = low;
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
    {
      // if upgrade counter fires...
      if (!(--(actual->ucount)))
	{
	  // upgrade actual state...
	  actual->state = nextState (tmp->level);
	  // upgrade alarm state...
	  actual->alarm = actual->state->level;
	  // ...initialize actual state thresholds...
	  actual->ucount = (actual->state)->sup;
	  actual->dcount = (actual->state)->low;
	  // ...and alarm level
	  actual->alevel = actual->dcount;
	}
    }
    // value <= instant threshold
  else 
    // reset upgrade counter to sup threshold
    actual->ucount = tmp->sup;

  // valutate alarm level...
  // ...if 0, downgrade alarm state to actual state...
  if (!actual->alevel)
    {
      actual->alarm = actual->state->level;
      actual->alevel = actual->dcount;
    }
  // ...otherwise... 
  else
    // ...if actual state < alarm state...
    if (actual->state->level < actual->alarm)
      // ...decrease alarm level
      actual->alevel--;

  return (actual->state)->level;
}

stateLst  
runStateBCD (actualStatePtr actual, const int value)
{
  stateDscPtr tmp = actual->state;

  // value > istant threshold
  if (value > tmp->il)
    {
      // reset downgrade counter to low threshold
      actual->dcount = tmp->low;
      // if upgrade counter fires...
      if (!(--(actual->ucount)))
	{
	  // upgrade actual state...
	  actual->state = nextState (tmp->level);
	  // upgrade alarm state...
	  actual->alarm = actual->state->level;
	  // ...and initialize state thresholds...
	  actual->ucount = (actual->state)->sup;
	  actual->dcount = (actual->state)->low;
	  // ...and alarm level
	  actual->alevel = actual->dcount;
	}
    }
  else 
    {
      // value <= previuos instant threshold
      if (value <= (prevState (tmp->level))->il)
	{
	  //reset upgrade counter to sup threshold
	  actual->ucount = tmp->sup;
	  // if downgrade counter fires...
	  if (!(--(actual->dcount)))
	    {
             // downgrade actual state...
	     actual->state = prevState (tmp->level);
	     // ...and set corrisponding thresholds
	     actual->ucount = (actual->state)->sup;
	     actual->dcount = (actual->state)->low;	      
	    }
	}
      // previous instant threshold < value <= instant threshold
      else 
	{
          // reset upgrade counter to sup threshold
          actual->ucount = tmp->sup;	  
          // reset downgrade counter to low threshold
          actual->dcount = tmp->low;	  
	}
    }

  // valutate alarm level...
  // ...if 0, downgrade alarm state to actual state...
  if (!actual->alevel)
    {
      actual->alarm = actual->state->level;
      actual->alevel = actual->dcount;
    }
  // ...otherwise... 
  else
    // ...if actual state < alarm state...
    if (actual->state->level < actual->alarm)
      // ...decrease alarm level
      actual->alevel--;

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
    {
      // if downgrade counter fires...
      if (!(--(actual->dcount)))
	{
	  // downgrade actual state...
	  actual->state = prevState (tmp->level);
	  // ...and set corrisponding thresholds
	  actual->ucount = (actual->state)->sup;
	  actual->dcount = (actual->state)->low;
	}
    }
  else 
    // reset downgrade counter to low threshold
    actual->dcount = tmp->low;
  return (actual->state)->level;
}
