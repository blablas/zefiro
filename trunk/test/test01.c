#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "connectPLC.h"

typedef struct 
{
  int data[MAX];
  int f;
  int l;
  int min;
  int max;
} plcData; 

void
printList (plcData *buffer)
{
  int i;

  for (i = 0; i < MAX - 1; i++)
    printf ("%d,", buffer->data[i]);
  printf ("%d\n", buffer->data[MAX - 1]);
}

void
insList (plcData *buffer, const int value)
{
  // new value for last index
  buffer->l = (buffer->l + 1) % MAX; 
  if (buffer->data[buffer->f] > 0)
    // MAX element queue 
    if (buffer->l == buffer->f) 
      // new value form first index
      buffer->f = (buffer->f + 1) % MAX;
  // insert value
  buffer->data[buffer->l] = value;
}

void 
initList (plcData *buffer)
{
  int i;

  for (i = 0; i < MAX; i++)
    buffer->data[i] = -1;
  buffer->f = 0;
  buffer->l = MAX - 1;
  buffer->min = 0;
  buffer->max = 0;
}

int 
main (void)
{
  int i = 1 , vp;

  daveConnection *dc; 
  daveInterface *di;
  plcData wlist;

  initList (&wlist);
 
  if (!plcConnect (i, &dc, &di))
    {
      for (i = 0; i < 22; i++)
	{
	  daveReadBytes (dc, daveDB, DAREA, 0, DBLEN, NULL);
	  vp = daveGetU16At (dc, 2);
	  printf ("Vp: %d\n", vp);
          insList (&wlist, vp);
          printList (&wlist);
	  usleep (1000000);
	}
      plcDisconnect (&dc, &di);
    }

  return vp;
}
