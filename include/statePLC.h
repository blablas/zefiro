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
typedef enum {A, B, C, D, E} plcState;

// PLC's data
typedef struct 
{
  plcState state;
  // actual state's upgrade level
  int ucount;
  // actual state's downgrade level
  int dcount;
} plcData; 

// state function prototype
typedef plcState (*plcRunState) (plcData *, const int);

// PLC's state structure
typedef struct 
{
  // instant state level
  int il;
  // state's upgrade threshold
  int sup; 
  // state's downgrade threshold
  int low;
  // state function
  plcRunState newState;
} plcStateDsc;

// actual state function implementation
extern plcState runStateA (plcData *plc, const int data);
extern plcState runStateB (plcData *plc, const int data);
extern plcState runStateC (plcData *plc, const int data);
extern plcState runStateD (plcData *plc, const int data);
extern plcState runStateE (plcData *plc, const int data);

// PLC's state table (forward declaration)
extern const plcStateDsc plcTable[];
#endif
