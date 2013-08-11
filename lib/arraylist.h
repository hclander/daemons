/*
 * arraylist.h
 *
 *  Created on: 11/08/2013
 *      Author: juanky
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

typedef struct arraylist {

	void *list;
	int size;
	int len;


} arraylist_t;

typedef arraylist_t *arraylist_p;

typedef void * arraylist_find_function(void *search, void *data);

arraylist_t *arraylist_create(int size);

void arraylist_destroy(arraylist_t *al, int freeItems);

int arraylist_add(arraylist_t *al, void *data);

void *arraylist_get(arraylist_t *al, int idx);

int arraylist_indexOf(arraylist_t *al, void *data);

void *arraylist_find(arraylist_t *al,arraylist_find_function findFunc,void *search);

void arraylist_delete(arraylist_t *al, int freeItem);


int arraylist_getLen(arraylist_t *al);


#endif /* ARRAYLIST_H_ */
