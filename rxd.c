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
#include "lib/hashinttable.h"
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



int sendAck(int sckt, long serialNumber,struct sockaddr *addTo, socklen_t toLen ) {
	int result = false;
	unsigned char buf[TRANS_MAX_BUFF_SIZE];
	frm_cmd_ack_t ack;
	size_t len = sizeof(ack);


	if ( frame_encode_ack(serialNumber,0,&ack,&len)){
		len =sizeof(buf);
		if (frame_encode_transport(0,&ack,sizeof(ack),buf,&len )) {
			sendto(sckt,buf,len,0,addTo,toLen);
			result = true;
		}
	}

	return result;
}

int sendAckOld(int sckt, long serialNumber,struct sockaddr *addTo, socklen_t toLen ) {
	int result = false;

	unsigned char ackOld[]={0xEF,0x00,0x00,0x00,0xEF};

	sendto(sckt,ackOld,sizeof(ackOld),0,addTo,toLen);

	result = true;

    return result;
}

//Very quick & dirty UDP Server
//TODO: Improving performance using  select()/poll()  and maybe multiprocess or multithreading.

int runUDPserver() {

	int sckt;
	int n;
	socklen_t fromLen;
	struct sockaddr_in server, from;

	hashint_table_t *ht;
	time_t *lastTime;

	char buf[TRANS_MAX_BUFF_SIZE];
	DB_T *db;

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

    ht = hashint_table_create(0);



	fromLen = sizeof(from);

	db = db_create("localhost","Yii","juanky","demo");

	if (!db_connect(db)) {
		die("Error connecting database");
	}

	//TODO: select()/poll()

	while(!terminate) {  // loop until terminate

	   n = recvfrom(sckt,buf,TRANS_MAX_BUFF_SIZE,0,(struct sockaddr *)&from,&fromLen);

	   // De momento no señalizamos nada
	   //sendto(sckt,MSG_OK,sizeof(MSG_OK),0,(struct sockaddr *)&from,fromLen);
	   //

	   LOG_F_N("Incoming [%s:%u (%d)]",inet_ntoa(from.sin_addr),ntohs(from.sin_port),n);




	   if ( !(lastTime=hashint_table_get(ht,ntohl(from.sin_addr.s_addr)))){
		   lastTime = malloc(sizeof(time_t));
		   *lastTime = time(NULL);
		   hashint_table_add(ht,ntohl(from.sin_addr.s_addr),lastTime);
	   }


	   if (frame_test_transport(buf,n)) {

		   transport_buf_p trans =(transport_buf_p) buf;

		   // Ahora sólo guaradamos la carga util . El resto se guarda como campos de la tabla
		   mydb_insert_transport_frame(db,ntohl(from.sin_addr.s_addr),ntohs(from.sin_port),ntohl(trans->header.sn),buf+TRANS_PREAMBLE_SIZE,n-TRANS_OVERLOAD);

		   // TODO Esto hay que cambiarlo;
		   // Mandar ACKs
		   int size = n-TRANS_OVERLOAD;
		   size_t len;
		   int offset = TRANS_PREAMBLE_SIZE;
		   long sn = ntohl(trans->header.sn);

		   len = size;
		   if (frame_test_gps(buf+offset,&len)) {
			   offset +=len;
			   size   -=len;

			   // TODO implementar una pequeña lista para solo enviar
			   // acks cada X tiempo

			   lastTime = hashint_table_get(ht,ntohl(from.sin_addr.s_addr));

			   if ( time(NULL)-(*lastTime)>120)
				   sendAckOld(sckt,sn,(struct sockaddr *)&from,fromLen);
		   }

		   len = size;
		   if (frame_test_cnx(buf+offset,&len)) {
			   offset +=len;
			   size   -=len;

			   sendAckOld(sckt,sn,(struct sockaddr *)&from,fromLen);
		   }

		   // Otros test;


	   }

	   *lastTime = time(NULL);

	   //TODO: Guardar los datos a la BBDD

	}

	close(sckt);
	hashint_table_destroy(ht,true);
	db_destroy(db);

	return EXIT_SUCCESS;
}

void doTestAndDie() {

	unsigned char buf[] = {0x00,0x00,0x17,0x01,0x02,0x03,0x04,0x11,0x00,0x00,0x00,0x01,0x2a,0x33,0x90,0x08,0x98,0x58,0x91,0x3c,0x8C,0x51,0xFA,0x32,0x41,0xAC}; //BB

	unsigned char ogps[]= {0x00,0x00,0x1A,0x01,0x02,0x03,0x04,0x13,0x11,0x00,0x01,0x00,0x00,0xAA,0x86,0xEF,0x72,0x01,0xA8,0x58,0x9A,0x8B,0x29,0x05,0x4A,0x8D,0x27,0x3E,0xA4};
	// 09/08/2013 12:33:46	41,48642	-5,718267	11	1	1	48	1	0	1	1	7	Zamora	CL-605	ZAMORA	1	0	1	3	39 = 0x27	1

	frm_cmd_gps_t gps;
	frm_cmd_gps_p pgps;

	transport_buf_p trans;

	time_t epoch;

	char strTime[50];

	unsigned char chkSum=0;


    if (frame_test_transport(ogps,sizeof(ogps))) {

    	struct tm *ptime;

    	trans = (transport_buf_p) ogps;
    	frm_cmd_gps_old_p opgps = (frm_cmd_gps_old_p) trans->data;

    	epoch= time(NULL);
    	ptime = gmtime(&epoch);

    	printf("Old Gps size = " ,sizeof(frm_cmd_gps_old_t));

    	opgps->data.asWord = ntohs(opgps->data.asWord);

    	printf("Datos trama Gps:\n"
    			    "\tCMD: 0x%02x\n"
    				"\tLEN: %d\n"
    			    "\tSEQ_L: 0x%04X\n"
    			    "\tSEQ_S: 0x%04X\n"
    				"\tLAT: %.4f\n"
    				"\tLON: %.4f\n"
    				"\tBEAR: %d\n"
    				"\tFIX: %d\n"
    				"\tHDOP: %d\n"
    				"\tKNOTS: %d\n"
    				"\tSPEED: %.3f kmph\n"
    				"\tExten. %d \n"
    				"\tSize: %d \n"
    			 	"\tIgnition: %d\n"
    			    "\tTurnon: %d\n"
    				"\tTIME: %04d-%02d-%02d %02d:%02d:%02d UTC\n"
    			    "\n"
    				,opgps->cmd
    				,opgps->len
    				,ntohs(opgps->seq_l)
    				,ntohs(opgps->seq_s)
    				,GPS_DECODE_LOC(opgps->lat_sign,opgps->lat_deg,ntohs(opgps->lat_min))
    				,GPS_DECODE_LOC(opgps->data.parts.lon_sign,opgps->lon_deg,ntohs(opgps->lon_min))
    				,GPS_DECODE_OLD_BEARING(opgps->data.parts.bear2,opgps->bear1,opgps->bear0)
    				,opgps->data.parts.fix
    				,opgps->hdop
    				,opgps->knots
    				,GPS_DECODE_SPEED(opgps->knots)
    				,opgps->ext
    				,opgps->size
    				,opgps->data.parts.ign
    				,opgps->turnon
    				,((ptime->tm_year+1900) & ~0x3) | opgps->year
    				,opgps->month
    				,opgps->day
    				,opgps->hour
    				,opgps->data.parts.mins
    				,opgps->data.parts.secs
    				);

    }







	trans = (transport_buf_p) buf;

	printf("Buff size = " ,sizeof(buf));

	printf("Transport Size 0x%08X 0x%08X %d\n",trans->header.length,ntohs(trans->header.length),ntohs(trans->header.length));



	int dataLen = ntohs(trans->header.length);

	/*
		for (int i=0; i< dataLen-1 ;i++) {
		   chkSum ^= buf[TRANS_HEADER_SIZE+i];
		}
    */

	DB_T *db = db_create("localhost","Yii","jcmendez","locatel");
	if (!db_connect(db)) {
		die("Error connecting database");
	}

	if ( frame_test_transport(buf,sizeof(buf)) ) {

		printf("Transport check ok\n");
		mydb_insert_transport_frame(db,0,0,0,buf,sizeof(buf));


		frm_cmd_gps_t gps;
		size_t gpsLen = sizeof(gps);

		if (frame_decode_gps(buf+TRANS_DATA_OFFSET,dataLen-TRANS_HEADER_SERIAL_SIZE,&gps,&gpsLen)) {

			mydb_insert_gps_subframe(db,1,1,&gps,gpsLen);

		}

	}

	//db_disconnect(db);
	db_destroy(db);

	printf("Datos transporte:\n"
			"\tStart\n"
			"\t\tLit. Endian: %d\n"
			"\t\tCRC : %d\n"
			"\t\tVersion: %d\n"
			"\t\tSerial: 0x%08X\n"
			"\t\tLength: %d\n"
			"\t\tChkSum:  0x%02X\n"
			"\t\tChkSum2: 0x%02X\n"
			"\t\tVerSum:  0x%02X\n"
			, trans->header.start.flags.lendian
			, trans->header.start.flags.crc
			, trans->header.start.flags.version
			, ntohl(trans->header.sn)
			, ntohs(trans->header.length)
			, trans->data[ntohs(trans->header.length)-TRANS_HEADER_SERIAL_SIZE-1]
			, buf[TRANS_HEADER_SIZE+dataLen-1]
			, chkSum
			);


	printf("\nData: ");
	for (int i=0; i< dataLen; i++) {

		printf(" %02X",trans->data[i]);
	}
    printf("\n");


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
			,ntohs(pgps->seq_l)
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

	int exitCode;

	//doTestAndDie();

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

	exitCode=runUDPserver();

	return exitCode;

	//return EXIT_SUCCESS;
}
