#include <math.h>
#include "tools.h"


void rnd_init(){
	srand(time(NULL));
}

//Aleatorio 0 - Max-1
int rnd(int max) {
	return rand()%max;
}
// Aleatorio 0 - Max ambos incluidos
int rnd0(int max){
	return rand()%(max+1);
}

//Aleatorio 1 - Max
int rnd1(int max){
	return rand()%max+1;
}
// Aleatorio low - max  ambos incluidos
int rndAB(int low, int max) {

	return rand()%(max-low+1)+low;
}

void freeIf(void *ptr){
	if(ptr)
		free(ptr);
}

void freeAndNull(void **ptr) {
	if (*ptr) {
		free(*ptr);
		*ptr=NULL;
	}
}


// Varias funciones muy cutres de coordenadas




long lon2x(float lon) {

	long x;

	x = round(EARTH_OFFSET + EARTH_RADIUS * lon * DEG_RAD);

	return x;

}

long lat2y(float lat) {
	long y;

	y = round(EARTH_OFFSET - EARTH_RADIUS * log(1+sin(lat * DEG_RAD ))/(1-sin(lat*DEG_RAD)/2));

	return y;

}

float x2lon(long x) {
	float lon;

	lon = -180 + 0.0000006705522537 * x;

	return lon;
}

float y2lat(long y) {
	float lat;

	double e = 2.7182818284590452353602875;
	double a = 268435456;
	double b = 85445659.4471;
	double c = 0.017453292519943;

	lat = asin(pow(e,(2*a/b-2*y/b))/(pow(e,(2*a/b-2*y/b))+1)-1/(pow(e,(2*a/b-2*y/b))+1))/c;

	return lat;

}
