/*
 * framedecoder_list.h
 *
 *  Created on: 14/08/2013
 *      Author: jcmendez
 */

#ifndef FRAMEDECODER_LIST_H_
#define FRAMEDECODER_LIST_H_
#include <stdlib.h>

typedef int(*framedecoder_func_t)(unsigned char *src, size_t srcLen, void *dst, size_t *dstLen);

#define T framedecoder_list_t
typedef struct T *T;


T fdl_create();

void fdl_destroy(T *fdl);

int fdl_register_func(T fdl,int cmd,framedecoder_func_t function);

int fdl_unregister_func(T fdl,int cmd);

framedecoder_func_t fdl_get_func(T fdl,int cmd);

int fdl_get_count(T fdl);

void *fld_get_list(T fdl);


#undef T

#endif /* FRAMEDECODER_LIST_H_ */
