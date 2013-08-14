/*
 * framedecoder_list.h
 *
 *  Created on: 14/08/2013
 *      Author: jcmendez
 */

#ifndef FRAMEDECODER_LIST_H_
#define FRAMEDECODER_LIST_H_

typedef int(*framedecoder_function_t)(unsigned char *src, size_t srcLen, void *dst, size_t *dstLen);

// Lo defino así porque seguramente necesitaré añadir mas campos especificos a la estructura framedecoder_list_t
// para su gestion u otras cosas...

typedef struct {

	 hashint_table_t decoders;


} framedecoder_list_t;

typedef framedecoder_list_t *framedecoder_list_p;

framedecoder_list_t *fdl_create();

void fdl_destroy(framedecoder_list_t *fdl);


int fdl_register_function(framedecoder_list_t *fdl,int cmd,framedecoder_function_t function);

int fdl_unregister_function(framedecoder_list_t *fdl,int cmd);

framedecoder_function_t fdl_get_function(framedecoder_list_t *fdl,int cmd);

#endif /* FRAMEDECODER_LIST_H_ */
