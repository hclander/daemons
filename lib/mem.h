/*
 * mem.h
 *
 *  Created on: 19/08/2013
 *      Author: jcmendez
 */

#ifndef MEM_H_
#define MEM_H_
#include <stdlib.h>

void *mem_malloc(size_t size);

void *mem_calloc(size_t count, size_t size) ;
void *mem_realloc(void *ptr, size_t size);

void mem_free(void *ptr);


#define ALLOC(size)	mem_malloc(size)
#define CALLOC(count, size) mem_calloc(count,size)

#define  NEW(p) ((p) = ALLOC(sizeof *(p)))
#define NEW0(p) ((p) = CALLOC(1,sizeof *(p)))
#define FREE(p) ((void)(mem_free((p)),(p) = NULL))
#define RESIZE(p, size) ((p) = mem_realloc((p),size))
#endif /* MEM_H_ */
