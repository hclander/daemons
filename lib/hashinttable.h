/*
 * hashinttable.h
 *
 *  Created on: 12/08/2013
 *      Author: jcmendez
 */

#ifndef HASHINTTABLE_H_
#define HASHINTTABLE_H_

// Hash<long,void *> implementation

#define HASHINT_DEFAULT_SIZE  10

#define T hashint_table_t

typedef struct T *T;

T hashint_table_create(int size);
void hashint_table_destroy( T *ht, int freeItems);
int hashint_table_add(T ht,long key, void *data);
int hashint_table_delete(T ht, long key, int freeItem);
void *hashint_table_get(T ht,long key);
int hashint_table_getSize(T ht);
int hashint_table_getLen(T ht);
int hashint_table_indexOf(T ht, long key);

#undef T

#endif /* HASHINTTABLE_H_ */
