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

typedef struct {
	long key;
	void *data;

} hashint_inner_item_t;

typedef struct {
	hashint_inner_item_t *items;
	int size;
	int len;


} hashint_table_t;



typedef hashint_table_t *hashint_table_p;

hashint_table_t * hashint_table_create(int size);
void hashint_table_destroy( hashint_table_t *ht, int freeItems);
int hashint_table_add(hashint_table_t *ht,long key, void *data);
int hashint_table_delete(hashint_table_t *ht, long key, int freeItem);
void *hashint_table_get(hashint_table_t *ht,long key);
int hashint_table_getSize(hashint_table_t *ht);
int hashint_table_getLen(hashint_table_t *ht);
int hashint_table_indexOf(hashint_table_t *ht, long key);


#endif /* HASHINTTABLE_H_ */
