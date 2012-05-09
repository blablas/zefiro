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
#ifndef STATEPLC_H
#define STATEPLC_H

// PLC's state enumeration
typedef enum {A, B, C, D, E, Z} stateLst;

typedef struct actualState * actualStatePtr;
typedef struct stateDsc * stateDscPtr;

// state function prototype
typedef stateLst (*runState) (actualStatePtr , const int);

// PLC's state descriptor
struct stateDsc
{
  // state level
  stateLst level; 
  // instant state level
  int il;
  // state's upgrade threshold
  int sup; 
  // state's downgrade threshold
  int low;
  // state transition function
  runState newState;  
};

// PLC's actual state
struct actualState
{
  // actual state descriptor
  stateDscPtr state;
  // alarm level
  stateLst alarm;
  // actual state's upgrade level
  int ucount;
  // actual state's downgrade level
  int dcount;
  // alarm's downgrade level
  int alevel;
}; 

// PLC's state functions table
extern const runState runStateTbl[];

// PLC's state table
extern stateDscPtr stateDscTbl[];

// actual state function implementation
extern stateLst runStateA (actualStatePtr plc, 
			   const int data);
extern stateLst runStateBCD (actualStatePtr plc, 
			     const int data);
extern stateLst runStateE (actualStatePtr plc, 
			   const int data);
extern stateLst runStateDummy (actualStatePtr plc, 
			       const int data);

// PLC's actual state initialization and finalization functions
extern actualStatePtr initActualState (const stateLst state);
extern void disposeActualState (actualStatePtr actual);

// PLC's state descriptor table initialization and finalization functions
extern stateDscPtr add2StateDscTbl (const stateLst pos, 
				       const stateLst level, 
				       const int il, 
				       const int sup, 
				       const int low);
extern void resetStateDscTbl (void);
#endif
