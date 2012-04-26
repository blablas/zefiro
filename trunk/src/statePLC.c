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
	  asPtr->ucount = (asPtr->state)->sup;
	  asPtr->dcount = (asPtr->state)->low;
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
add2StateDscTbl (const stateLst pos, const stateLst label, const int il, const int sup, const int low, runState newState)
{
  stateDscPtr dsPtr = NULL;
 
  dsPtr = malloc (sizeof (struct stateDsc));
  if (dsPtr)
    {
      dsPtr->label = label;
      dsPtr->il = il;
      dsPtr->sup = sup;
      dsPtr->low = low;
      dsPtr->newState = newState;
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
	  actual->state = nextState (tmp->label);
	  // ...and initialize actual state thresholds
	  actual->ucount = (actual->state)->sup;
	  actual->dcount = (actual->state)->low;
	}
    }
    // value <= instant threshold
    else 
      // reset upgrade counter to sup threshold
      actual->ucount = tmp->sup;
  return (actual->state)->label;
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
	  actual->state = nextState (tmp->label);
	  // ...and initialize state thresholds
	  actual->ucount = (actual->state)->sup;
	  actual->dcount = (actual->state)->low;
	}
    }
  else 
    {
      // value <= previuos instant threshold
      if (value <= stateDscTbl[tmp->label - 1]->il)
	{
	  //reset upgrade counter to sup threshold
	  actual->ucount = tmp->sup;
	  // if downgrade counter fires...
	  if (!(--(actual->dcount)))
	    {
             // downgrade actual state...
	     actual->state = prevState (tmp->label);
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
  return (actual->state)->label;
}

stateLst  
runStateDummy (actualStatePtr actual, const int value)
{
  return Z;
}

stateLst  
runStateE (actualStatePtr actual, const int value)
{
  stateDscPtr tmp = actual->state;

  // value < istant threshold
  if (value < tmp->il)
    // if downgrade counter fires...
    if (!(--(actual->dcount)))
      {
	// downgrade actual state...
	actual->state = prevState (tmp->label);
	// ...and set corrisponding thresholds
	actual->ucount = (actual->state)->sup;
	actual->dcount = (actual->state)->low;
      }
  return (actual->state)->label;
}
