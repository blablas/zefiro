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
      // ...and initialize new actual state's thresholds
      actual->ucount = (actual->state)->sup;
      actual->dcount = (actual->state)->low;
    }
}

inline void 
downgradeState (actualStatePtr actual, stateDscPtr tmp)
{
  // reset upgrade counter to sup threshold cause we're looking for upgrade!
  actual->ucount = tmp->sup;
  // if downgrade counter fires...
  if (!(--(actual->dcount)))
    {
       // downgrade actual state...
       actual->state = prevState (tmp->level);
       // ...and initialize new actual state's thresholds
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
add2StateDscTbl (const stateLst level, 
		 const int vp, const int vmax, const int vmin, const int iv, 
		 const int sup, const int low, 
		 const int evt, const int ntv, const int smp)
{
  stateDscPtr dsPtr = NULL;
 
  dsPtr = malloc (sizeof (struct stateDsc));
  if (dsPtr)
    {
      dsPtr->level = level;
      dsPtr->vp = vp;
      dsPtr->vmax = vmax;
      dsPtr->vmin = vmin;
      dsPtr->iv = iv;
      dsPtr->sup = sup;
      dsPtr->low = low;
      dsPtr->newState = runStateTbl[level];
      if (stateDscTbl[level] != NULL)
	free (stateDscTbl[level]);
      stateDscTbl[level] = dsPtr;
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
runStateA (actualStatePtr actual, const int vp, const int vmax, const int vmin, const int iv)
{  
  stateDscPtr tmp = actual->state;

  // at least one of the value > corresponding istant threshold --> instant state upgraded!
  if ((vp > tmp->vp) ||
      (vmax > tmp->vmax) ||
      (vmin > tmp->vmin) ||
      (iv > tmp->iv))
    // let's see if we can upgrade activity state...
    upgradeState (actual, tmp);
  // instant state has not changed!
  else 
    resetState (actual, tmp);
  return (actual->state)->level;
}

stateLst  
runStateBCD (actualStatePtr actual, const int vp, const int vmax, const int vmin, const int iv)
{
  stateDscPtr tmp = actual->state;

  // at leat on of the values > corresponding istant threshold --> instant state upgraded!
  if ((vp > tmp->vp) ||
      (vmax > tmp->vmax) ||
      (vmin > tmp->vmin) ||
      (iv > tmp->iv))
    // let's see if we can upgrade activity state...
    upgradeState (actual, tmp);
  else 
    {
      // all values <= corresponding previuos instant threshold --> instant state downgraded!
      if ((vp <= (prevState (tmp->level))->vp) &&
	  (vmax <= (prevState (tmp->level))->vmax) &&
	  (vmin <= (prevState (tmp->level))->vmin) &&
	  (iv <= (prevState (tmp->level))->iv))
        // let's see if we can downgrade activity state...
	downgradeState (actual, tmp);
      // instant state has not changed!
      else 
	resetState (actual, tmp);
    }
  return (actual->state)->level;
}

stateLst  
runStateDummy (actualStatePtr actual, const int vp, const int vmax, const int vmin, const int iv)
{
  return (actual->state)->level;
}

stateLst  
runStateE (actualStatePtr actual, const int vp, const int vmax, const int vmin, const int iv)
{
  stateDscPtr tmp = actual->state;

  // all values < istant threshold --> instant state downgraded!
  if ((vp < tmp->vp) &&
      (vmax < tmp->vmax) &&
      (vmin < tmp->vmin) &&
      (iv < tmp->iv))
    // let's see if we can downgrade activity state...
    downgradeState (actual, tmp);
  // instant state has not changed!
  else 
    resetState (actual, tmp);
  return (actual->state)->level;
}

