/*
 * decodemanager.h
 *
 *  Created on: 19/08/2013
 *      Author: jcmendez
 */

#ifndef DECODEMANAGER_H_
#define DECODEMANAGER_H_

typedef struct {
	int    rx_id;
	int    loc_id;
	time_t time;

} decode_context_t;

#define T decodemanager_t

typedef T *T;


T decodemanager_create();
void decodemanager_destroy(T *dm);

void decodemanagee_init(T dm);

void decodemanager_finish(T dm);

int decodemanager_decode(T db, void *buff, size_t size, int *count);


#undef T
#endif /* DECODEMANAGER_H_ */
