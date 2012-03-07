#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "statePLC.h"

const plcStateDsc plcTable[] =
{
    /*  il     sup     low      newState*/
      { 1, 	2, 	0, 	runStateA }, 
      { 5, 	3, 	5,  	runStateB }, 
      { 10, 	4, 	4,  	runStateB }, 
      { 15, 	5, 	3,  	runStateB }, 
      { 20, 	0, 	2,  	runStateE }
};

int 
main (void)
{
  int i, value;
  plcData plc = {A, plcTable[A].sup, plcTable[A].low};

  for (;;)
    {
      printf ("value (ctrl-c to quit): ");
      scanf ("%d", &value);
      plcTable[plc.state].newState (&plc, value);
      printf ("state: %d, level: %d, ucount: %d, dcount: %d\n", plc.state, plcTable[plc.state].il, plc.ucount, plc.dcount); 
    }
  return 0;
}
