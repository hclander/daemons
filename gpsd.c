#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>

#include "lib/log.h"
#include "frames.h"
#include "mydb.h"

int verboseLevel= 1;
bool keepAsDaemon=true;
int timeLapse=5;
bool terminate = false;

const char *TAG="GPSd";


void help() {
	printf("gpsd v1.0 Daemon de recepcion de tramas\n\n"
			 "Syntaxis:\n"
			 "\n\tgpsd [ -h | --help ] [-l | --lapse] [ -v | --verbose  < 0 | 1 | 2 > ]  \n"
			 "\n\twhere:\n"
			 "\t -h | --help     : Shows this help.\n"
			 "\t -v | --verbose  : Sets the verbose level( 0 Error , 1 Notice, 2  Debug )\n"
		);
}


void parseArgs(int argc, char **argv) {

	 bool noArgs    = true;
	 static struct option longOptions[] = {
	    		{"help",0,0,'h'},
	    		{"lapse",1,0,'l'},
	    		{"verbose",1,0,'v'},
	    		{0,0,0,0}
	    };

		//Parseo de parametros;
	    int c;

	    while ( (c = getopt_long(argc,argv,"hl:v:n",longOptions,NULL)) != -1) {

	    	if (noArgs)
	    		noArgs = false;

	    	switch(c) {
	    		case 'l':
	    			timeLapse = atoi(optarg);
	    			break;
				case 'v':
					verboseLevel = atoi(optarg);
					break;
				case 'n':
					keepAsDaemon = false;
					break;
				default:
					help();
					exit(EXIT_FAILURE);
	    	}
	    }

	    if (noArgs) {
	    	help();
	    	exit(EXIT_FAILURE);
	    }
}

void initLogs() {

	INITLOG(TAG);
	// Para desactivar niveles de logs
	if (verboseLevel==0)
		LOG_LEVEL(LOG_ERR);
	else if (verboseLevel==1)
		LOG_LEVEL(LOG_NOTICE);
	else
		LOG_LEVEL(LOG_DEBUG);
}

void die(char *msg) {
  LOG_E(msg);
  exit(EXIT_FAILURE);
}

void signalHandler(int signal) {

	switch (signal) {

	case SIGHUP:
		LOG_W("Hungup signal received. Do something interesting");
		//TODO: Do something interesting.
		break;
	case SIGTERM:
		LOG_W("Teminate signal received. Quitting");

		terminate = true;

		//exit(EXIT_SUCCESS);
		break;

	}
}


int processData(RES_T *res, DB_T *dbw,DB_T *dbr) {

	ROW_T *row;
	int rx_id,loc_id;
	frm_cmd_gps_t gps;
	unsigned char *buf;
	size_t len,gpsLen;
	int count;
	int status=2;
	int ok;

	if (res_isClosed(res)) {
		LOG_E("Recordset is closed");
		return EXIT_FAILURE;
	}


	loc_id = 1;
	gpsLen = sizeof(gps);

	while (!row_isClosed(row=res_next(res))  ) {

		rx_id = atoi(row_getFieldValue(row, row_getFieldIndex(row,"id")));
		len   = atoi(row_getFieldValue(row, row_getFieldIndex(row,"len")));

		buf = row_getFieldValue(row, row_getFieldIndex(row,"data"));


		//FIXME La gestion de decodermanager no está funcionando...
		// actualmenet solo sirve para contar cuantas posibles subtramas pudiera haber..
		//
		ok=frame_decodermanager_decode(buf,len,&count);

		if (count>0) {

			if (ok)
				status = 1;

			//FIXME Cambiar esto que solo esta para guardar tramas GPS y tiene que ser generico

			if ( frame_decode_gps(buf,len,&gps,&gpsLen) ) {

				if ( !mydb_insert_gps_subframe(dbw,rx_id,loc_id,&gps,gpsLen) ) {
					status = 3;
				}
			}

			// FIXME Megachapuzada para grabar las tramas 0x15... Niños no hagais esto en casa. Va en contra de toda logíca y raciocinio...
			// es mas ver esto puede producir trastornos cerebrales severos...
			if ( frame_decode_rally_gps_old(buf,len,NULL,NULL)) {

				int recCount;

				recCount=mydb_insert_rally_old_gps_frame(dbw,rx_id,loc_id,buf,len);

				LOG_F_D("Trama 0x15 (loc_id = %d, rx_id=%d) : Decodificados %d registros",loc_id,rx_id,recCount);

				if (!recCount)
					status = 3;

			}



		} else {
			status = -1;
			LOG_E("No se pudo decodificar nada");
		}



//		if ( frame_decode_gps(buf,len,&gps,&gpsLen) ) {
//
//			if ( mydb_insert_gps_subframe(dbw,rx_id,loc_id,&gps,gpsLen) )
//				mydb_update_transport_frame_status(dbr,rx_id,1);
//			else
//				mydb_update_transport_frame_status(dbr,rx_id,3);
//
//		} else {
//			mydb_update_transport_frame_status(dbr,rx_id,2);
//			LOG_E("No es una trama gps");
//		}

		mydb_update_transport_frame_status(dbr,rx_id,status);

	}

	row_destroy(row);

	return EXIT_SUCCESS;

	//	int num_fields;
	//	MYSQL_FIELD *fields;
	//
	//	MYSQL_ROW row;
	//
	//	MYSQL *conWrite;
	//
	//
	//	num_fields = mysql_num_fields(res);
	//
	//	fields = mysql_fetch_fields(res);
	//
	//	//TODO: Leer los datos descomprimirlos y guardarlos en la BBDD
	//	while ((row = mysql_fetch_row(res)) != NULL ) {
	//
	//		 for(int i = 0; i < num_fields; i++) {
	//
	//			LOG_F_D("%s = %s\t", fields[i].name, row[i]?row[i]:"NULL");
	//
	//		 }
	//
	//	}
}

int runMonitor() {

	//TODO: Necesitamos doble conexions  una para la lectura y otra para la escritura
	DB_T *dbr;
	DB_T *dbw;

	RES_T *res;

	LOG_N("Inicializando monitor...");

	dbr = db_create("localhost","Yii","user","entrar");
	dbw = db_create("localhost","Yii","user","entrar");


	db_connect(dbr);
	if (!db_isConnected(dbr)) {
		LOG_E(db_getError(dbr));
		db_destroy(dbr);
		return EXIT_FAILURE;
	}


	db_connect(dbw);

	if (!db_isConnected(dbw)) {
		LOG_E(db_getError(dbw));
		db_destroy(dbw);
		db_destroy(dbr);
		return EXIT_FAILURE;
	}

	frame_decodermanager_init();

   while(!terminate) {

	   // Sospecho que tras un tiempo la conexion con la bbdd se cierra....
	   if (!db_chkConnection(dbr)) {
		   LOG_E("Read database is closed. Trying to reconnect");
		   db_connect(dbr);
	   }

	   // Sospecho que tras un tiempo la conexion con la bbdd se cierra....
	   if (!db_chkConnection(dbw)) {
		   LOG_E("Write database is closed. Trying to reconnect");
		   db_connect(dbw);
	   }

	   LOG_D("Checking for undecoded frames...");
	   res = mydb_select_undecoded_transport_frames(dbr);

	   if (res_getRowCount(res)>0) {
		   LOG_F_N("Processing %d new frames..",res_getRowCount(res));
		   processData(res,dbw,dbr);
		   LOG_N("... done");
	   }

	   res_destroy(res);
//	   if ( mysql_query(conRead,sql) ) {
//
//		   LOG_E(mysql_error(conRead));
//
//
//	   } else {
//
//		   res = mysql_use_result(conRead);
//
//		   if (res != NULL) {
//
//
//			   processGpsData(res);
//
//			   mysql_free_result(res);
//
//		   }
//
//
//	   }

	   LOG_F_D("...All done. Sleeping %d",timeLapse);
	   sleep(timeLapse);   // Wait 5 seconds;
   }

	db_destroy(dbr);
	db_destroy(dbw);
   //mysql_close(conRead);
	frame_decodermanager_finish();

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	struct sigaction action;
	int exitCode;
	parseArgs(argc,argv);
	initLogs();


	if (keepAsDaemon) {

		LOG_N("Trying to keep as a daemon...");

		if (daemon(false,false)<0) {
			LOG_E(strerror(errno));
			LOG_W("... continuing daemonless");

		} else {

			LOG_N("... Success ");
		}

	}

	memset(&action,0,sizeof(action));
	action.sa_handler =signalHandler;


	sigaction(SIGHUP,&action,NULL);
	sigaction(SIGTERM,&action,NULL);

//	signal(SIGHUP,signalHandler); /* catch hangup signal */
//	signal(SIGTERM,signalHandler); /* catch kill signal */



	exitCode = runMonitor();

	return exitCode;


	//return EXIT_SUCCESS;
}

