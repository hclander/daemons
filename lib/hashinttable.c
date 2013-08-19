/*
 * hashinttable.c
 *
 *  Created on: 12/08/2013
 *      Author: jcmendez
 */

#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "hashinttable.h"

#define T hashint_table_t

typedef struct {
	long key;
	void *data;

} hashint_inner_item_t;

struct hashint_table_t {
	hashint_inner_item_t *items;
	int size;
	int len;

};

T hashint_table_create(int size) {
	T ht;
	NEW(ht);

	if(size>0)
		ht->size = size;
	else
		ht->size = HASHINT_DEFAULT_SIZE;

	ht->items = calloc(ht->size,sizeof(hashint_inner_item_t));
	ht->len = 0;

	return ht;
}

void hashint_table_destroy( T *ht, int freeItems) {
	if(ht && *ht) {
		if (freeItems) {
			for (int i=0; i<(*ht)->len; i++)
				free((*ht)->items[i].data);
		}
		free((*ht)->items);
		FREE(*ht);
	}
}

int hashint_ensure_size(T ht) {
	if (ht->len == ht->size) {  // Se ha agotado el espacio
		ht->size+=HASHINT_DEFAULT_SIZE;
		ht->items=realloc(ht->items,ht->size*sizeof(hashint_inner_item_t));
	}
	return 0;
}

int hashint_table_add(T ht,long key, void *data) {
	int idx;

	idx=hashint_table_indexOf(ht,key);

	if (idx!=-1)
		return idx;

	hashint_ensure_size(ht);
	ht->items[ht->len].key = key;
	ht->items[ht->len].data = data;
	ht->len++;

	return ht->len-1;
}

int hashint_table_indexOf(T ht, long key) {

	for(int i=0; i<ht->len;i++)
		if (ht->items[i].key==key)
			return i;

	return -1;
}

void *hashint_table_get(T ht,long key) {
	int idx;

	idx = hashint_table_indexOf(ht,key);

	if (idx!=-1)
		return ht->items[idx].data;

	return NULL;
}



int hashint_table_delete(T ht, long key, int freeItem) {
	int idx=-1;

	idx=hashint_table_indexOf(ht,key);

	if (idx!=-1) {
		if (freeItem)
			free(ht->items[idx].data);
		ht->len--;
		if (idx!=ht->len) {
			//Dememos mover los elementos
			memmove(&ht->items[idx],&ht->items[idx+1],(ht->len-idx)*sizeof(hashint_inner_item_t));
		}
	}

	return idx;
}



int hashint_table_getSize(T ht) {
	return ht->size;
}

int hashint_table_getLen(T ht) {
	return ht->len;
}
