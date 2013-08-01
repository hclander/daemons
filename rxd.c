#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

// includes for socket

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib/log.h"
#include "frames.h"
#include "mydb.h"

#define UDP_DEFAULT_PORT 4490

int udpPort =UDP_DEFAULT_PORT;
int verboseLevel= 1;
bool keepAsDaemon=true;

bool terminate = false;

const char *TAG="RXd";

void help() {
  printf("rxd v1.0 Daemon de recepcion de tramas\n\n"
		 "Syntaxis:\n"
		 "\n\trxd [ -h | --help ] [ -p | --port <udp_port> ] [ -v | --verbose  < 0 | 1 | 2 > ]  \n"
		 "\n\twhere:\n"
		 "\t -h | --help     : Shows this help.\n"
		 "\t -p | --port     : UDP port to listen.\n"
		 "\t -v | --verbose  : Sets the verbose level( 0 Error , 1 Notice, 2  Debug )\n"
	);
}



void parseArgs(int argc, char **argv) {

	 bool noArgs    = true;
	 static struct option longOptions[] = {
	    		{"help",0,0,'h'},
	    		{"port",1,0,'p'},
	    		{"verbose",1,0,'v'},
	    		{0,0,0,0}
	    };

		//Parseo de parametros;
	    int c;

	    while ( (c = getopt_long(argc,argv,"hp:v:n",longOptions,NULL)) != -1) {

	    	if (noArgs)
	    		noArgs = false;

	    	switch(c) {
				case 'p':
					udpPort = atoi(optarg);
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

int runUDPserver() {

	int sckt;
	int n;
	socklen_t fromLen;
	struct sockaddr_in server, from;

	char buf[MAX_TRANS_BUFF_SIZE];

	LOG_D("Iniciando servidor UDP");

	memset(buf,0,sizeof(buf));

	LOG_D("Creando socket");
	sckt = socket(AF_INET,SOCK_DGRAM,0);

	if (sckt<0) {

		LOG_E(strerror(errno));

		return EXIT_FAILURE;
	}

	memset(&server,0,sizeof(server));

    server.sin_family= AF_INET;
    server.sin_port=htons(udpPort);
    server.sin_addr.s_addr=htonl(INADDR_ANY);

	LOG_D("Bindando socket");

    if (bind(sckt,(struct sockaddr *)&server,sizeof(server))<0) {

	   LOG_E(strerror(errno));

	   return EXIT_FAILURE;

    }

    LOG_F_N("Server listening on port %u",udpPort);

	fromLen = sizeof(from);

	while(!terminate) {  // loop until terminate

	   n = recvfrom(sckt,buf,MAX_TRANS_BUFF_SIZE,0,(struct sockaddr *)&from,&fromLen);

	   // De momento no señalizamos nada
	   //sendto(sckt,MSG_OK,sizeof(MSG_OK),0,(struct sockaddr *)&from,fromLen);
	   //

	   LOG_F_N("Incoming [%s:%u (%d)]",inet_ntoa(from.sin_addr),ntohs(from.sin_port),n);

	   //TODO: Guardar los datos a la BBDD

	}

	close(sckt);

	return EXIT_SUCCESS;
}

int doTestAndDie() {

	char buf[] = {0x00,0x00,0x17,0x01,0x02,0x03,0x04,0x11,0x00,0x00,0x00,0x01,0x2a,0x33,0x90,0x08,0x98,0x58,0x91,0x3c,0x8C,0x51,0xFA,0x32,0x41};

	frm_cmd_gps_t gps;
	frm_cmd_gps_p pgps;

	transport_buf_p trans;

	time_t epoch;

	char strTime[50];

	char chkSum=0;

	trans = (transport_buf_p) buf;

	printf("Buff size = " ,sizeof(buf));

	printf("Transport Size 0x%08X 0x%08X %d\n",trans->header.length,ntohs(trans->header.length),ntohs(trans->header.length));

	for (int i=0; i<trans->header.length + sizeof(trans->header.sn);i++) {
		   chkSum ^= buf[TRANS_CHKBUFF_OFFSET+i];
		}

	printf("Datos transporte:\n"
			"\tStart\n"
			"\t\tLit. Endian: %d\n"
			"\t\tCRC : %d\n"
			"\t\tVersion: %d\n"
			"\t\tChkSum:  0x%02X\n"
			"\t\tVerSum:  0x%02X\n"


			, trans->header.start.flags.lendian
			, trans->header.start.flags.crc
			, trans->header.start.flags.version
			, ntohl(trans->header.sn)
			, ntohs(trans->header.length)
			, trans->data[ntohs(trans->header.length)]
			, chkSum
			);



	printf("Transport SN 0x%08X 0x%08X \n",trans->header.sn,ntohl(trans->header.sn));

    printf("Sizeof gps Struct = %u\n",sizeof(gps));

	pgps = (frm_cmd_gps_p) trans->data;

	epoch = ntohl(pgps->time.epoch);

	strftime(strTime,sizeof(strTime),"%F %T",gmtime(&epoch));

	printf("Datos trama Gps:\n"
		    "\tCMD: 0x%02x\n"
		    "\tSEQ: 0x%08X\n"
			"\tLAT: %.4f\n"
			"\tLON: %.4f\n"
			"\tBEAR: %d\n"
			"\tFIX: %d\n"
			"\tHDOP: %d\n"
			"\tKNOTS: %d\n"
			"\tSPEED: %.3f kmph\n"
			"\tTIME: %d %s\n"
		    "\n"
			,pgps->cmd
			,ntohl(pgps->seq)
			,GPS_DECODE_LOC(pgps->lat_sign,pgps->lat_deg,ntohs(pgps->lat_min))
			,GPS_DECODE_LOC(pgps->lon_sign,pgps->lon_deg,ntohs(pgps->lon_min))
			,GPS_DECODE_BEARING(pgps->bearing)
			,pgps->fix
			,pgps->hdop
			,pgps->knots
			,GPS_DECODE_SPEED(pgps->knots)
			,ntohl(pgps->time.epoch)
			,strTime
			);

	die("End of test");
}

int main(int argc, char **argv) {

	doTestAndDie();

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

	return runUDPserver();

	//return EXIT_SUCCESS;
}
