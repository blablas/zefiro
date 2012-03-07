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
#include "statePLC.h"

plcState 
runStateA (plcData *cur, const int value)
{  
  plcStateDsc plc = plcTable[cur->state];

  // value > istant threshold
  if (value > plc.il)
    {
      // if upgrade counter fires...
      if (!(--(cur->ucount)))
	{
	  // upgrade state...
	  ++(cur->state);
	  // ...and set corrisponding thresholds
	  cur->ucount = plcTable[cur->state].sup;
	  cur->dcount = plcTable[cur->state].low;
	}
    }
    // value <= instant threshold
    else 
      // reset upgrade counter to sup threshold
      cur->ucount = plc.sup;
  return cur->state;
}

plcState 
runStateB (plcData *cur, const int value)
{
  plcStateDsc plc = plcTable[cur->state];

  // value > istant threshold
  if (value > plc.il)
    {
      // reset downgrade counter to low threshold
      cur->dcount = plc.low;
      // if upgrade counter fires...
      if (!(--(cur->ucount)))
	{
	  // upgrade state...
	  ++(cur->state);
	  // ...and set corrisponding thresholds
	  cur->ucount = plcTable[cur->state].sup;
	  cur->dcount = plcTable[cur->state].low;
	}
    }
  else 
    {
      // value < previuos instant threshold
      if (value < plcTable[(cur->state) - 1].il)
	{
	  //reset upgrade counter to sup threshold
	  cur->ucount = plc.sup;
	  // if downgrade counter fires...
	  if (!(--(cur->dcount)))
	    {
             // downgrade state...
	     --(cur->state);
	     // ...and set corrisponding thresholds
	     cur->ucount = plcTable[cur->state].sup;
	     cur->dcount = plcTable[cur->state].low;	      
	    }
	}
      // previous instant threshold < value <= instant threshold
      else 
	{
          // reset upgrade counter to sup threshold
          cur->ucount = plc.sup;	  
          // reset downgrade counter to low threshold
          cur->dcount = plc.low;	  
	}
    }
  return cur->state;
}

plcState 
runStateC (plcData *cur, const int value)
{
  return cur->state;
}

plcState 
runStateD (plcData *cur, const int value)
{
  return cur->state;
}

plcState 
runStateE (plcData *cur, const int value)
{
  plcStateDsc plc = plcTable[cur->state];

  // value < istant threshold
  if (value < plc.il)
    // if downgrade counter fires...
    if (!(--(cur->dcount)))
      {
	// downgrade state...
	--(cur->state);
	// ...and set corrisponding thresholds
	cur->ucount = plcTable[cur->state].sup;
	cur->dcount = plcTable[cur->state].low;
      }
  return cur->state;
}
