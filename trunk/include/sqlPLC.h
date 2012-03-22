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
#ifndef SQLPLC_H
#define SQLPLC_H
#include <mysql.h>
#include <time.h>

#define SELECT_PLC_DTA 		"SELECT id, net, msk, ip, dsc FROM registry where id = ?"
#define SELECT_ENABLE_LEVELS 	"SELECT id, vp, vmax, vmin, iv, up, do, hys FROM levels where en > 0"
#define UPDATE_LIVE_DTA		"UPDATE liveValues SET vp = ?, vmax = ?, vmin = ?, iv = ?, ts = now(), lvl = ? WHERE id = ?"
extern int sqlConnect (MYSQL **conn, my_bool autoCommit);
extern void sqlDisconnect (MYSQL **conn);
extern int sqlCloseStmt (MYSQL_STMT **stmt);
extern int sqlPrepareStmt (MYSQL *conn, MYSQL_STMT **stmt, const char *query);
extern int sqlBindParam (MYSQL_STMT *stmt, MYSQL_BIND *bind);
extern int sqlBindResult (MYSQL_STMT *stmt, MYSQL_BIND *bind);
extern int sqlExecStmt (MYSQL_STMT *stmt);
#endif
