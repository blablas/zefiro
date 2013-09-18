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
#include <getopt.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include "statePLC.h"
#include "sqlPLC.h"
#include "threadPLC.h"
#include "connectPLC.h"

#define EXIT_SUCCESS 	0
#define EXIT_FAILURE 	1
#define MAXWIND 	170

// plc's status
typedef enum {UNK, RUN, STP, ERR} pStatus; 
typedef struct pData * pDataPtr;

// plc's data structure
struct pData {
    // mobile average data windows
    int *vList;
    // plc's ip address
    char ip[15];
    // plc's MPI address
    int mpi,
    	// plc's RACK
    	rack,
    	// plc's SLOT
    	slot,
    	// plc's id
    	id,
	// signal waited for next activation
	sig,
	// first element in data windows
	first,
	// last element in data windows
	last,
	// current min for wind speed in mobile windows
        min,
	// current max for wind speed in mobile windows
        max,
	// current average for wind speed in mobile windows
        avg,
	// anemometer 1 data
        vp1,
	// anemometer 2 data
        vp2,
	// weight (spreader + load)
        wgt,
	// height (spreader)
        hgt,
	// position (driver's cab)
        pos;
    // plc's info
    unsigned char nfo[2];
		  // plc's alarms
//		  alm[19];
    // anemometer actual state
    actualStatePtr act;
};

extern void daemonize (const char *pidfile);
extern void processData (pDataPtr plc, const int value);
extern pDataPtr* initPlcsData (MYSQL * conn, int *len);
extern void freePlcsData (pDataPtr *dta, const int len);
extern int initWindLevels (MYSQL *conn);
extern int initWindLevelParam (MYSQL *conn);
extern int setBackLog (MYSQL *conn, const bool enable);
extern int setLiveDta (MYSQL *conn, const int unsigned id, const bool enable);
extern int setPlcState (MYSQL *conn, const int unsigned id, const unsigned int status);
extern void* doWork (void *argv);

extern int lfp;
extern char *pidfile;
extern unsigned int MIN, 
       MAX, 
       NRETRY;
#endif

// ALARMS REGISTY (bits: 0 - 150) 
#define A0	Sezionatore portale non inserito
#define A1	Sovratensione motori portale
#define A2	Termico armatura motori portale
#define A3	Guasto regolatore campo portale
#define A4	Mancanza consensi portale
#define A5	Termico ventilazione drivers sollevamento e portale
#define A6	Allarme minimo campo portale
#define A7	Emergenza SNAG
#define A8	Termico freno portale 
#define A9	Centrifugo sollevamento
#define A10	Allarme drive portale
#define A11	Termico soppressione sovratensione	
#define A12	Termico sincronismi sollevamento
#define A13	Finecorsa emergenza alto sollevamento
#define A14	Sezionatore sollevamento aperto
#define A15	Allarme drive sollevamento o fusibile armatura
#define A16	Allarme campo sollevamento
#define A17	Mancanza consensi sollevamento
#define A18	Emergenza generale
#define A19	Sovratensione motore sollevamento
#define A20	Allarme drive sollevamento
#define A21	Termico freni sollevamento
#define A22	Allarme convertitore braccio
#define A23	Mancanza consensi braccio
#define A24	Sezionatore braccio aperto
#define A25	Mancanza campo braccio
#define A26	Centrifugo braccio
#define A27	Finecorsa emergenza alto braccio
#define A28	Termico sincro braccio
#define A29	Termico sincronismi carrello
#define A30	Scatto fusibile armatura braccio
#define A31	Termico campo braccio
#define A32	Termico ventilatore drives braccio e carrello
#define A33	Termico freno braccio
#define A34	Allarme convertitore carrello
#define A35	Sezionatore carrello aperto
#define A36	Allarme carrello aperto
#define A37	Termico armatura motori carrello o sovratensione
#define A38	Mancansa consensi carrello
#define A39	Termico eccitazione carrello
#define A40	Emergenza pulpito braccio
#define A41	Motorizzato non inserito
#define A42	Termico 220v scaldiglie J01
#define A43	Allarme Cunei lato terra BASSI
#define A44	Allarme Cunei lato mare BASSI	
#define A45	Perni antiuragano lato terra BASSI
#define A46	Emergenza portale lato terra
#define A47	Sonda termica motori portale lato terra
#define A48	Sonda termica motori portale lato mare
#define A49	Finecorsa fine cavo media (raotativo o ultrasuoni)
#define A50	Bando cavo media
#define A51	Perni antiuragano lato mare BASSI
#define A52	Emergenza portale lato mare
#define A53	Emergenza pulpito braccio
#define A54	Giunto carrello su motore emergenza
#define A55	Sonda termica motore 1 carrello
#define A56	Sonda termica motore 2 carrello
#define A57	Emergenza carrello
#define A58	Termico pompa spreader
#define A59	Termico 220v QSVCO
#define A60	Termico proiettori carrello
#define A61	Termico sirena
#define A62	Termico scaldiglie e ventilatore QSVCO
#define A63	Termico motori emergenza carrello
#define A64	Termico motore 1 carrello
#define A65	Termico motore 2 carrello
#define A66	Termico freno 1 carrello
#define A67	Termico freni carrello
#define A68	Termico freno 2 carrello
#define A69	Termico trafo 110v QSVCO
#define A70	Termico 110v elettrovalvole QSVCO
#define A71	Termico 110v uscite QSVCO
#define A72	Termico 24v spreader
#define A73	Emergenza QSVCO
#define A74	Selezione carello in emergenza
#define A75	Termico 24v poltrona lato sx
#define A76	Termico 24v poltrona lato dx
#define A77	Termico 24v pannello cabina operatore
#define A78	Traversa non agganciata
#define A79	Emergenza da poltrona comandi
#define A80	Sezionatore linea MT aperto
#define A81	Interruttore MT trafo potenza aperto
#define A82	Protezione max corrente media tensione
#define A83	Protezione terra
#define A84	Protezione bassa tensione
#define A85	Allarme temperatura TM1 trafo potenza
#define A86	Allarme temperatura TM2 trafo ausiliari
#define A87	Centralina fumi cabina elettrica
#define A88	Antisurriscaldamento sollevamento
#define A89	Giunto sollevamento in posizione emergenza
#define A90	Centralina emergenza sollevamento in manuale
#define A91	Freni emergenza sollevamento non aperti
#define A92	Sonda termica motore sollevamento
#define A93	Scarrucolamento funi braccio
#define A94	Centralina emergenza barccio in manuale
#define A95	Freni emergenza braccio non aperti
#define A96	Giunto motore braccio in emergenaza
#define A97	Sonda termica motore braccio
#define A98	Emergenza cassetta manutenzione sollevamento
#define A99	Emergenza cassetta manutenzione braccio
#define A100	Emergenza quadro A02
#define A101	Interruttore 380v aperto
#define A102	Selezione motore braccio in emergenaza
#define A103	Ok centraline cunei
#define A104	Allarme avvolgicavo media
#define A105	Ok ventilatore sala argani
#define A106	Allarme centralina arpione
#define A107	Allarme centralina TLS
#define A108	Allarme freni emergenza braccio
#define A109	Allarme freno emergenza sollevamento
#define A110	Termico scardiglie
#define A111	Selezione motore sollevamento in emergenza
#define A112	Allarme luci aeree
#define A113	Preallarme vento
#define A114	Allarme vento
#define A116	Mancata apertura freno braccio
#define A117	Mancata apertura freni carrello
#define A118	Mancata apertura freno sollevamento
#define A119	Motore sollevamento non ni coppia
#define A120	Motore braccio non in coppia
#define A121	Mancato rallentamento carrello lato mare
#define A122	Errore di posizione tamburi sollevamento > TR-ENCODERS <
#define A123	Guasto strumento celle
#define A124	Massimo carico - GRU IN RALLENTAMENTO
#define A125	Sovraccarico - STOP SOLLEVAMENTO
#define A126	Massimo carico singola fune - STOP SOLLEVAMENTO
#define A127	Sovraccarico su fune LATO MARE SX
#define A128	Sovraccarico su fune LATO TERRA SX
#define A129	Sovraccarico su fune LATO TERRA DX
#define A130	Sovraccarico su fune LATO MARE DX
#define A131	Scatto pressostato su fune LATO MARE SX
#define A132	Scatto pressostato su fune LATO TERRA SX
#define A133	Scatto pressostato su fune LATO TERRA DX
#define A134	Scatto pressostato su fune LATO MARE DX
#define A135	Funi in bado braccio - STOP DISCESA BRACCIO	
#define A136	Guasto manipolatore comando alza - ABBASSA BRACCIO CABINA BOOM
#define A137	Attivo by-pass cella portapersone
#define A138	Errore alta velocità Hoist in zona sollevamento > TR-ENCODERS <
#define A139	Errore sincronizzazione fc rotativo Hoist con > TR-ENCODERS <
#define A144	Errore alta velocità braccio in zona rallentamento > TR-ENCODERS <
#define A145	Errore alta velocità braccio centrifugo da > TR-ENCODERS <
#define A146	Errore extra alto braccio da > TR-ENCODERS <

// INFO REGISTRY
#define I0	Interruttore motorizzato chiuso
#define I1	Presa comandi da braccio
#define I2	Presa comandi da cabina operatore
#define I3	Presa comandi da cabina checker
#define I7	Zappe antiuragano basse
#define I8	Pompa spreader in moto
#define I9	Spreader in 20 piedi
#define I10	Spreader in 40 piedi
#define I11	Twist aperti
#define I12	Twist chiusi
#define I13	Spreader appoggiato
#define I14	Twin spreader alto
#define I15	Twin spreader basso

