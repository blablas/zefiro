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
#ifndef ZEFIRO_H
#define ZEFIRO_H
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <pwd.h> 
#include <signal.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include "statePLC.h"
#include "sqlPLC.h"
#include "threadPLC.h"
#include "connectPLC.h"

typedef struct pData * pDataPtr;

struct pData {
    int id;
    char ip[15];
};

extern void daemonize (const char *pidfile);
extern pDataPtr* initPlcsData (MYSQL * conn, int *len);
extern int initWindLevels (MYSQL *conn);
extern int setBackLog (MYSQL *conn, const bool enable);

extern int lfp;
extern char *pidfile;
#endif
