#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "readPLC.h"

#define	MAX	10

typedef struct 
{
  int wind[MAX];
  int first;
  int last;
} windList; 

void
printList (windList *buffer)
{
  int i;

  for (i = 0; i < MAX; i++)
    printf ("%d,", buffer->wind[(buffer->first + i) % MAX]);
  printf ("\n");
}

void
insList (windList *buffer, const int value)
{
  int tmp;
  tmp = buffer->wind[buffer->first];
  buffer->last = (buffer->last + 1) % MAX; 
  buffer->wind[buffer->last] = value;
  if ((buffer->last == buffer->first) && tmp != -1)
    buffer->first = (buffer->first + 1) % MAX;
  printf ("(first, last): (%d, %d)\n", buffer->first, buffer->last);
}

void 
initList (windList *buffer)
{
  int i;

  for (i = 0; i < MAX; i++)
    buffer->wind[i] = -1;
  buffer->first = 0;
  buffer->last = MAX - 1;
}

int 
main (void)
{
  int i = 1, vp = -1;

  daveConnection *dc; 
  daveInterface *di;
  windList wlist;

  initList (&wlist);
 
  if (!plcConnect (i, &dc, &di))
    {
      for (i = 0; i < 22; i++)
	{
	  vp = plcRead (dc, daveFlags, 0, 100, 2, NULL);
	  printf ("Vp: %d\n", vp);
          insList (&wlist, vp);
          printList (&wlist);
	  usleep (1000000);
	}
      plcDisconnect (&dc, &di);
    }

  return vp;
}

