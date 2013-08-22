/*
 * test.c
 *
 *  Created on: 21/08/2013
 *      Author: jcmendez
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <endian.h>
#include <string.h>
#include <math.h>

#include "lib/bytebuffer.h"


void prueba1() {

	FILE *fout;

	bytebuffer_t bb = NULL;

	    bb = bb_create();

	    bb_allocate(bb,200);

	    //bb_setOrder(bb,BB_ORDER_LITTLE_ENDIAN);

	    bb_put(bb,3);

	    bb_putInt16(bb,32);

	    bb_putInt32(bb_putInt16(bb,100),1500);

	    bb_putInt64(bb,1523252352324);
	    bb_putInt32(bb,-1);


	    bb_putDouble(bb,M_PI);

	    bb_putFloat(bb,2356.3252);

	    fout = fopen("test.bin","w");
	    if (fout) {

	    	fwrite(bb_getBuffer(bb),1,bb_getPosition(bb),fout);

	    	fclose(fout);
	    }


	    bb_rewind(bb);


	    // ERROR -> En general no es buena idea usar los bb_getXXX  como argumentos de una funcion pues hay que tener
	    //  en cuenta que en C los argumentos se pushean en orden inverso con lo que los valores obtenidos pueden no coincider con lo esperado...
	    //   Seguramnete no se ha entendido nada pero yo me entiendo..

	    printf("\nDatos:\nByte (%d)\nShort (%d)\nShort (%d)\nInt (%d)\nLong (%ld)\nInt (%d)\nDouble (%f)\nFloat (%f)\n\n",
	    		bb_get(bb), bb_getInt16(bb), bb_getInt16(bb),bb_getInt32(bb),bb_getInt64(bb),bb_getInt32(bb),bb_getDouble(bb),bb_getFloat(bb)
	    	);


	    bb_rewind(bb);
	    //ERROR
	    printf("\nDatos:\nFloat (%f)\nDouble (%f)\nInt (%d)\nLong (%ld)\nInt (%d)\nShort (%d)\nShort (%d)\nByte (%d)\n\n",
	    		bb_get(bb), bb_getInt16(bb), bb_getInt16(bb),bb_getInt32(bb),bb_getInt64(bb),bb_getInt32(bb),bb_getDouble(bb),bb_getFloat(bb)
	    	);

	    bb_rewind(bb);
	    // Deberia estar bien
	    printf("\nDatos:\nFloat (%f)\nDouble (%f)\nInt (%d)\nLong (%ld)\nInt (%d)\nShort (%d)\nShort (%d)\nByte (%d)\n\n",
	        		bb_getFloat(bb),bb_getDouble(bb),bb_getInt32(bb),bb_getInt64(bb),bb_getInt32(bb), bb_getInt16(bb), bb_getInt16(bb),bb_get(bb)
	        	);


	    printf("\nPi = (%.20f) (%.20lf)\n\n",M_PI,M_PI);

	    printf("\nPi = (%.20Lf)\n\n",(long double)M_PI);

	    bb_destroy(&bb);

	    if (!bb)
	    	printf("Ok. bb is NULL\n\n");

}

void prueba2() {
	unsigned char buf[] = {0x00,0x00,0x17,0x01,0x02,0x03,0x04,0x11,0x00,0x00,0x00,0x01,0x2a,0x33,0x90,0x08,0x98,0x58,0x91,0x3c,0x8C,0x51,0xFA,0x32,0x41,0xAC}; //BB
	unsigned char ogps[]= {0x00,0x00,0x1A,0x01,0x02,0x03,0x04,0x13,0x11,0x00,0x01,0x00,0x00,0xAA,0x86,0xEF,0x72,0x01,0xA8,0x58,0x9A,0x8B,0x29,0x05,0x4A,0x8D,0x27,0x3E,0xA4};
	unsigned char cnx[] = {0x00,0x00,0x1B,0x00,0x00,0xFF,0xFF,0x06,0x14,0x33,0x35,0x31,0x37,0x37,0x37,0x30,0x34,0x33,0x38,0x34,0x38,0x35,0x30,0x34,0x32,0x05,0x00,0x01,0x10,0x1D};

	bytebuffer_t bb;
	int sig,deg,min;

	bb = bb_create();
	bb_wrap(bb,buf,sizeof(buf));
	char imei[16];

	//bb_setOrder(bb,BB_ORDER_BIG_ENDIAN);  Por defecto el ByteBuffer se inicializa como BIG_ENDIAN

	// Ejemplo de parseo mediante ByteBuffer

	printf("Datos Buf:\n");

	printf("\tStart: %02X\n",bb_get(bb));
	printf("\tSize : %d\n",bb_getInt16(bb));
	printf("\tSN   : %08X\n",bb_getInt32(bb));
	printf("\t\tCmd: %d\n",bb_get(bb));
	printf("\t\tSeqS: %d\n",bb_getInt16(bb));
	printf("\t\tSeqL: %d\n",bb_getInt16(bb));

	// Lat
	deg = bb_get(bb);
	sig = deg >>7;
	min = bb_getInt16(bb);
	deg &= 0x7F;

	printf("\t\tLat: %f\n",(deg+min/60000.0) * (1+sig*-2));

	// Lon
	deg = bb_get(bb);
	min = bb_getInt16(bb);
	sig = bb_getByIdx(bb,bb_getPosition(bb)) >>7; // TODO hacer funciones bb_peek

	printf("\t\tLon: %f\n",(deg+min/60000.0) * (1+sig*-2));

	printf("\t\tBear: %d\n",(bb_get(bb)&0x7F)*4);

	printf("\t\tKnots: %d\n",bb_get(bb));
    printf("\t\tFix: %d\n",bb_getByIdx(bb,bb_getPosition(bb)) >>7);
	printf("\t\tHdop: %d\n",(bb_get(bb)& 0x7f));

	printf("\t\tEpoch: %d\n",bb_getInt32(bb));
	printf("\tEnd : %02X\n\n",bb_get(bb));

	bb_destroy(&bb);

	bb=bb_wrap(bb_create(),cnx,sizeof(cnx));

	printf("Datos CNX:\n");

	printf("\tStart: %02X\n",bb_get(bb));
	printf("\tSize : %d\n",bb_getInt16(bb));
	printf("\tSN   : %08X\n",bb_getInt32(bb));
	printf("\t\tCmd: %d\n",bb_get(bb));
	printf("\t\tLen: %d\n",bb_get(bb));

	memset(imei,0,sizeof(imei));

	bb_getToBuff(bb,imei,0,15);
	printf("\t\tImei: %s\n",imei);
	printf("\t\tImei Crc: %02X\n",bb_get(bb));
	printf("\t\tO.M: %02X\n",bb_get(bb));
	printf("\t\tS.V: %d\n",bb_getInt16(bb));
	printf("\t\tC.R: %d\n",bb_get(bb));
	printf("\tEnd : %02X\n\n",bb_get(bb));

	bb_destroy(&bb);

}

void prueba3() {

	bytebuffer_t bb1,bb2;

	char buf1[100];
	char buf2[100];

	for (int i=0; i<100; i++) {
		buf2[i]=buf1[i]=i;
	}

	bb1=bb_wrap(bb_create(),buf1,sizeof(buf1));
	bb2=bb_wrap(bb_create(),buf2,sizeof(buf2));

	// pruebas compare

	if (bb_equals(bb1,bb2)) {

		printf(" bb1 y bb2 son iguales\n");
	}

	bb_get(bb2); // consumimos 1 bytes

	printf(" bb1 es %s que bb2\n", bb_compare(bb1,bb2)<0?"menor":"mayor");


	bb_rewind(bb2);

	bb_putByIdx(bb2,50,0);

	printf(" bb1 es %s que bb2\n", bb_compare(bb1,bb2)<0?"menor":"mayor");


	bb_putByIdx(bb1,25,bb_getByIdx(bb2,25)-1);

	printf(" bb1 es %s que bb2\n", bb_compare(bb1,bb2)<0?"menor":"mayor");


	// pruebas compact , duplicate , slice

	bytebuffer_t dup,sl;

	bb_setPosition(bb2,35);

	dup=bb_duplicate(bb2);

	if (bb_equals(dup,bb2)) {
			printf(" dup y bb2 son iguales\n");
	}


	bb_setPosition(bb1,90);
	sl = bb_slice(bb1);
	printf(" Byte 0: %d (90)\n",bb_get(sl));
	printf(" Byte 5: %d (95)\n",bb_getByIdx(sl,5));
	bb_compact(bb1);

	bb_put(bb1,-1);
	bb_put(bb1,-1);


	bb_destroy(&bb1);
	bb_destroy(&bb2);
	bb_destroy(&dup);
	bb_destroy(&sl);

}

int main(int argc, char **argv) {


	printf("Chorizo de pruebas\n");


	prueba1();

	prueba2();

	prueba3();

	return 0;
}

