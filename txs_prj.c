/*
 * txs_prj.c
 *
 *  Created on: 06/08/2013
 *      Author: jcmendez
 */

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

#include <proj_api.h>
#include <project.h>


#include "lib/log.h"
#include "frames.h"
#include "lib/tools.h"


#define D_TO_R (M_PI/180)
#define R_TO_D (180/M_PI)

//42.211750,-8.650350
#define ORIG_LAT 42.211750
#define ORIG_LON -8.650350

#define UDP_DEFAULT_PORT 4490

int udpPort =UDP_DEFAULT_PORT;
int verboseLevel= 1;
int defaultLapse = 5;
char ip[20] = "127.0.0.1";
bool addCnxFrame = false;
bool terminate = false;
bool sendFile = false;

char *fileName;

#define MPS_TO_KNOTS  1.94384
#define MPS_TO_KMPH  3.6

#define MAX_MPS   36



const char *TAG="TXs-proj ";

void help() {
  printf("txs_prj v1.0 Very Simple frame sender simulator \n\n"
		 "Syntaxis:\n"
		 "\n\ttxs [ -h | --help ] [-i | --ip < ip_server > ][ -p | --port <udp_port> ] [-c | --cnx ] [ -l | --lapse ] [ -f | --file ] [ -v | --verbose  < 0 | 1 | 2 > ]  \n"
		 "\n\twhere:\n"
		 "\t -h | --help     : Shows this help.\n"
		 "\t -i | --ip       : Ip server.\n"
		 "\t -p | --port     : UDP port to listen.\n"
		 "\t -c | --cnx      : Append a connection frame.\n"
		 "\t -f | --file     : Send file content. \n"
		 "\t -l | --lapse    : Time lapse to send frames.\n"
		 "\t -v | --verbose  : Sets the verbose level( 0 Error , 1 Notice, 2  Debug )\n"
		 "\n"
	);
}

void die(char *msg) {
  fprintf(stderr,msg);
  exit(EXIT_FAILURE);
}


int sendFileContent(char *ip,int port, char *fileName) {
	char buff[TRANS_MAX_BUFF_SIZE];
	size_t transLen;
	int seq =0;
	int sckt;
	FILE *fin;
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

	fin = fopen(fileName,"r");

	if (!fin) {
		die("No se pudo abrir el fichero");

	}

	transLen=fread(buff,1,sizeof(buff),fin);

	sendto(sckt,buff,transLen,0,(struct sockaddr *)&server,sizeof(server));

	printf("Enviados %d bytes.\n",transLen);

	close(sckt);
	fclose(fin);

	return 0;
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
	char buff[TRANS_MAX_BUFF_SIZE];
	size_t transLen;
	int seq =0;
	int sckt;
	struct sockaddr_in server;
	time_t timestamp;


	PROJ *p;
	PROJ_XY xy;
	PROJ_LP lp;

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


	//Projection initialization

	//WGS84
	p = proj_initstr("proj=merc ellps=WGS84");

	lp.phi = ORIG_LAT * D_TO_R;
	lp.lam = ORIG_LON * D_TO_R;

	xy = proj_fwd(lp,p);

	printf("Empezando simulacion...(lapso=%ds)\n\n",lapse);

	while (true) {

		if (myTime<=0) {
			myTime = rnd1(30);
			bear = rnd0(359);
			mps  = rnd1(MAX_MPS);
		}

		d = mps * lapse;

		// Ojo origen de rumbo PI
		xy.y += round( d * cos(bear * D_TO_R));
		xy.x += round( d * sin(bear * D_TO_R));

		lp = proj_inv(xy,p);

		knots = mps * MPS_TO_KNOTS;

		int offset=0;
		transLen = sizeof(buff);


		lat = lp.phi* R_TO_D;
		lon = lp.lam * R_TO_D;

		timestamp = time(NULL);

		frame_encode_gps(seq++,bear,knots,lat,lon,1,rnd1(20),timestamp,buff,&transLen);

		offset +=transLen;

		if (addCnxFrame) {

			transLen = sizeof(buff) - offset;
			frame_encode_cnx("1234567890",0,1,0,buff+offset,&transLen);
			offset +=transLen;

		}

		transLen = sizeof(buff);

		if ( frame_encode_transport(1,NULL,offset,buff,&transLen) ) {

//			if (frame_test_transport(buff,transLen)) {
//				printf("Trama de transporte verificada\n");
//			}

			printf("Enviando: %d lat=%f lon=%f bear=%d knots=%d time=%d\n",seq,
					lat,lon,bear,knots,timestamp);

			sendto(sckt,buff,transLen,0,(struct sockaddr *)&server,sizeof(server));

		}

		myTime -=lapse;
		sleep(lapse);
	}

	proj_free(p);
	close(sckt);
	return 0;

}


void parseArgs(int argc, char **argv) {

	 bool noArgs    = true;
	 static struct option longOptions[] = {
	    		{"help",0,0,'h'},
	    		{"ip",1,0,'i'},
	    		{"port",1,0,'p'},
	    		{"file",1,0,'f'},
	    		{"cnx",0,0,'c'},
	    		{"lapse",1,0,'l'},
	    		{"verbose",1,0,'v'},
	    		{0,0,0,0}
	    };

		//Parseo de parametros;
	    int c;

	    while ( (c = getopt_long(argc,argv,"hcf:i:p:l:v:n",longOptions,NULL)) != -1) {

	    	if (noArgs)
	    		noArgs = false;

	    	switch(c) {
	    		case 'i':
	    			strcpy(ip,optarg);

	    			break;
				case 'p':
					udpPort = atoi(optarg);
					break;

				case 'f':
					sendFile = true;
					fileName = optarg;
					break;
				case 'c':
					addCnxFrame = true;
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


void doTestAndDie() {

	projPJ pj_merc, pj_latlon;

	PROJ *p;
	PROJ_XY xy;
	PROJ_LP lp;

	double lat,lon;

	pj_merc   =  pj_init_plus("+proj=merc +ellps=WGS84");
	pj_latlon =  pj_init_plus("+proj=latlong +ellps=WGS84");

	lat = ORIG_LAT;
	lon = ORIG_LON;

	lat *=D_TO_R; //DEG_TO_RAD;
	lon *=D_TO_R; //DEG_TO_RAD;

	pj_transform(pj_latlon,pj_merc,1,1,&lon,&lat,NULL);

	printf("Proj4   -> x=%lf,y=%lf\n",lon, lat);

	lp.phi=ORIG_LAT * D_TO_R;
	lp.lam=ORIG_LON * D_TO_R;




	//WGS84
	p = proj_initstr("proj=merc ellps=WGS84");

	xy = proj_fwd(lp,p);

	printf("Project -> x=%lf,y=%lf\n",xy.x,xy.y);

	//  x=-962913.595362,y=5165270.648993

	lp = proj_inv(xy,p);

	printf("Project -> lat=%lf,lon=%lf\n",lp.phi*R_TO_D,lp.lam*R_TO_D);


	exit(EXIT_SUCCESS);

}

void doSpiral() {


	PROJ *p;
	PROJ_XY xy,center,old;
	PROJ_LP lp;

	double radio,ang, delta,A,B,Eps;

	lp.phi=ORIG_LAT * D_TO_R;
	lp.lam=ORIG_LON * D_TO_R;



		//WGS84
	p = proj_initstr("proj=merc ellps=WGS84");

	center = proj_fwd(lp,p);

	//Spiral euclieds  r = a+b*Angulo

	delta = 1 * D_TO_R;
	ang = 0;
	A=1; B=2;
	Eps=0.0000001;

	while (true)  {

		radio = A+B*ang;

		xy.x = center.x + round( radio * cos(ang));
		xy.y = center.y + round( radio * sin(ang));

		lp = proj_inv(xy,p);

		if ( ( islessgreater(xy.x+Eps,old.x) && islessgreater(xy.y+Eps,old.y)))
				printf("Project -> lat=%lf,lon=%lf\n",lp.phi*R_TO_D,lp.lam*R_TO_D);

		old=xy;
		ang+=delta;
	}

}


int main(int argc, char **argv) {

	int exitCode;

	doSpiral();
	//doTestAndDie();

	parseArgs(argc,argv);

	if (sendFile)
		exitCode = sendFileContent(ip,udpPort,fileName);
	else
		exitCode = runSimulator(ip,udpPort);

	return exitCode;


}

