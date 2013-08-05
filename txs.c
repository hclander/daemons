#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <math.h>
// includes for socket

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib/log.h"
#include "frames.h"
#include "lib/tools.h"

#define UDP_DEFAULT_PORT 4490

int udpPort =UDP_DEFAULT_PORT;
int verboseLevel= 1;
int defaultLapse = 5;
char ip[20] = "127.0.0.1";

bool terminate = false;

#define MPS_TO_KNOTS  1.94384
#define MPS_TO_KMPH  3.6

#define MAX_MPS   36



const char *TAG="TXs";

void help() {
  printf("txs v1.0 Very Simple frame sender simulator \n\n"
		 "Syntaxis:\n"
		 "\n\ttxs [ -h | --help ] [-i | --ip < ip_server > ][ -p | --port <udp_port> ] [ -l | --lapse ] [ -v | --verbose  < 0 | 1 | 2 > ]  \n"
		 "\n\twhere:\n"
		 "\t -h | --help     : Shows this help.\n"
		 "\t -i | --ip       : Ip server.\n"
		 "\t -p | --port     : UDP port to listen.\n"
		 "\t -l | --lapse    : Time lapse to send frames.\n"
		 "\t -v | --verbose  : Sets the verbose level( 0 Error , 1 Notice, 2  Debug )\n"
	);
}

void die(char *msg) {
  fprintf(stderr,msg);
  exit(EXIT_FAILURE);
}


int runSimulator(char *ip, int port) {

	int bear;
	int knots;
	int mps=0;
	int myTime =0;
	int lapse=defaultLapse;
	long x, y, d;
	float lat, lon;
	frm_cmd_gps_t gps;
	char buff[MAX_TRANS_BUFF_SIZE];
	size_t transLen;
	int seq =0;
	int sckt;
	struct sockaddr_in server;

	// INIT SOCKET
	printf("Incializando...\n");
	memset(&server,0,sizeof(server));

	sckt= socket(AF_INET,SOCK_DGRAM,0);

	if (sckt<0) {
			die("Error en la creacion del socket");
	}

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(port);



	y = lat2y(42.22);
	x = lon2x(-8.65);
	printf("Empezando simulacion...(lapso=%ds)\n\n",lapse);

	while (true) {

		if (myTime<=0) {
			myTime = rnd1(30);
			bear = rnd0(359);
			mps  = rnd1(MAX_MPS);
		}

		d = mps * lapse;

		// Ojo origen de rumbo PI
		y += round( d * cos(bear * DEG_RAD));
		x += round( d * sin(bear * DEG_RAD));

		lat = y2lat(y);
		lon = x2lon(x);
		knots = mps * MPS_TO_KNOTS;

		gps.cmd = 0x11;
		gps.seq = seq++;
		gps.bearing = GPS_ENCODE_BEARING(bear);
		gps.knots = knots;
		gps.lat_sign = lat<0?1:0;
		gps.lat_deg = lat;
		gps.lat_min =htons(GPS_ENCODE_LOCMIN(lat));
		gps.lon_sign = lon<0?1:0;
		gps.lon_deg = lon;
		gps.lon_min = htons(GPS_ENCODE_LOCMIN(lon));

		gps.fix = 1;
		gps.hdop = rnd1(20);
		gps.time.epoch = htonl(time(NULL));


		transLen = sizeof(buff);

		if ( frame_encode_transport(1,&gps,sizeof(gps),buff,&transLen) ) {

//			if (frame_test_transport(buff,transLen)) {
//				printf("Trama de transporte verificada\n");
//			}

			printf("Enviando: %d lat=%f lon=%f bear=%d knots=%d time=%d\n",seq,
					lat,lon,bear,knots,ntohl(gps.time.epoch));

			sendto(sckt,buff,transLen,0,(struct sockaddr *)&server,sizeof(server));

		}

		myTime -=lapse;
		sleep(lapse);
	}

	return 0;

}


void parseArgs(int argc, char **argv) {

	 bool noArgs    = true;
	 static struct option longOptions[] = {
	    		{"help",0,0,'h'},
	    		{"ip",1,0,'i'},
	    		{"port",1,0,'p'},
	    		{"lapse",1,0,'l'},
	    		{"verbose",1,0,'v'},
	    		{0,0,0,0}
	    };

		//Parseo de parametros;
	    int c;

	    while ( (c = getopt_long(argc,argv,"hi:p:l:v:n",longOptions,NULL)) != -1) {

	    	if (noArgs)
	    		noArgs = false;

	    	switch(c) {
	    		case 'i':
	    			strcpy(ip,optarg);

	    			break;
				case 'p':
					udpPort = atoi(optarg);
					break;

				case 'l':
					defaultLapse = atoi(optarg);
					break;
				case 'v':
					verboseLevel = atoi(optarg);
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



int main(int argc, char **argv) {

	int exitCode;



	parseArgs(argc,argv);

	exitCode = runSimulator(ip,udpPort);

	return exitCode;

	//return EXIT_SUCCESS;
}
