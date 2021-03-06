#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "connectPLC.h"

#define	MAX 	10

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
  int mpi = 2, rack = 0, slot = 3, i, j, vp, res;
  unsigned char nfo[2];
  unsigned char alm[19];
  char *plc = "10.50.3.93";

  //daveSetDebug(daveDebugAll);
  daveConnection *dc; 
  plcData wlist;

  initList (&wlist);
 
  for (i = 0; i < 1000; i++) 
    {
      printf ("\nwaiting to connect...\n");
      if (!plcConnect (plc, mpi, rack, slot, &dc)) 
	{
	  //printf ("\ndaveReadBytes: press any key\n");
	  //getchar ();
	  if (res = daveReadBytes (dc, daveDB, DAREA, 0, DLEN + ALEN, NULL)) 
	    printf ("\nerror in daveReadBytes: %s\n", daveStrerror(res));
	  else 
	    {
	      vp = daveGetU16At (dc, 2);
	      for (j = 0; j < sizeof (nfo); j++)
		{
		  nfo[j] = daveGetU8At (dc, 14 + (j * sizeof (unsigned char)));
		  printf ("Nfo[%d]: %u(%d)\n", j, nfo[j], sizeof (nfo));
		}
	      for (j = 0; j < sizeof (alm); j++)
		{
		  alm[j] = daveGetU8At (dc, 18 + (j * sizeof (unsigned char)));
		  printf ("Alm[%d]: %u(%d)\n", j, alm[j], sizeof (alm));
		}
	      printf ("\t\tVp: %d\n", vp);
	      insList (&wlist, vp);
	      printList (&wlist);
	    }
	  plcDisconnect (dc);
	}
      usleep (1000000);
    }
  printf ("Press any key to disconnect");
  getchar ();
  return vp;
}
