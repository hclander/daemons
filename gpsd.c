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

bool terminate = false;

const char *TAG="GPSd";


void help() {
	printf("gpsd v1.0 Daemon de recepcion de tramas\n\n"
			 "Syntaxis:\n"
			 "\n\tgpsd [ -h | --help ] [ -v | --verbose  < 0 | 1 | 2 > ]  \n"
			 "\n\twhere:\n"
			 "\t -h | --help     : Shows this help.\n"
			 "\t -v | --verbose  : Sets the verbose level( 0 Error , 1 Notice, 2  Debug )\n"
		);
}


void parseArgs(int argc, char **argv) {

	 bool noArgs    = true;
	 static struct option longOptions[] = {
	    		{"help",0,0,'h'},
	    		{"verbose",1,0,'v'},
	    		{0,0,0,0}
	    };

		//Parseo de parametros;
	    int c;

	    while ( (c = getopt_long(argc,argv,"hv:n",longOptions,NULL)) != -1) {

	    	if (noArgs)
	    		noArgs = false;

	    	switch(c) {
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


int processGpsData(MYSQL_RES *res) {
	int num_fields;
	MYSQL_FIELD *fields;

	MYSQL_ROW row;

	MYSQL *conWrite;


	num_fields = mysql_num_fields(res);

	fields = mysql_fetch_fields(res);

	//TODO: Leer los datos descomprimirlos y guardarlos en la BBDD
	while ((row = mysql_fetch_row(res)) != NULL ) {

		 for(int i = 0; i < num_fields; i++) {

			LOG_F_D("%s = %s\t", fields[i].name, row[i]?row[i]:"NULL");

		 }

	}


	return EXIT_SUCCESS;
}

int runGpsMonitor() {

	//TODO: Necesitamos doble conexions  una para la lectura y otra para la escritura

   char sql[]="select * from rx_tbl where status=0";

   MYSQL *conRead;
   MYSQL_RES *res;

   conRead = mysql_init(NULL);

   //TODO: Comprobar posibles problemas de rendimiento
   // que es mejor abrir y cerrar la conexion
   // o dejarla siempre abierta.

   if ( !mysql_real_connect(conRead,"localhost","jcmendez","locatel","test",0,NULL,0) ) {

	   LOG_E(mysql_error(conRead));
	   return EXIT_FAILURE;
   }

   while(terminate) {

	   if ( mysql_query(conRead,sql) ) {

		   LOG_E(mysql_error(conRead));


	   } else {

		   res = mysql_use_result(conRead);

		   if (res != NULL) {


			   processGpsData(res);

			   mysql_free_result(res);

		   }


	   }


	   sleep(5);   // Wait 5 seconds;
   }

   mysql_close(conRead);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {

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

	signal(SIGHUP,signalHandler); /* catch hangup signal */
	signal(SIGTERM,signalHandler); /* catch kill signal */

	return runGpsMonitor();


	//return EXIT_SUCCESS;
}

