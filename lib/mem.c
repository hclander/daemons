/*
 * mem.c
 *
 *  Created on: 19/08/2013
 *      Author: jcmendez
 */

#include <stdlib.h>

/*
 * Wrappers a funciones de memoria
 * Por si queremos hacer alguna comprobacion o accion a mayores...
 *
 * Posible implementacion de seguimiento de punteros para depuracion
 */

void *mem_malloc(size_t size) {
	void *ptr;

	ptr =malloc(size);

	return ptr;
}

void *mem_calloc(size_t count, size_t size) {
	void * ptr;
	ptr = calloc(count,size);

	return ptr;
}

void *mem_realloc(void *ptr, size_t size) {

	ptr =realloc(ptr,size);

	return ptr;
}


void mem_free(void *ptr) {
	if (ptr)
		free(ptr);
}
