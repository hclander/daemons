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
