/*
 * framedecoder_list.c
 *
 *  Created on: 14/08/2013
 *      Author: jcmendez
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lib/hashinttable.h"
#include "framedecoder_list.h"



framedecoder_list_t *fdl_create() {
	framedecoder_list_t *fdl = calloc(1,sizeof(framedecoder_list_t));

	fdl->decoders = hashint_table_create(0);

	return fdl;
}


void fdl_destroy(framedecoder_list_t *fdl){
	if (fdl){
		hashint_table_destroy(fdl->decoders,false);
		free(fdl);
	}
}

//TODO Permitir registar varias funciones para un mismod cmd ??
int fdl_register_function(framedecoder_list_t *fdl,int cmd,framedecoder_function_t function) {

	if (fdl) {

		if (hashint_table_indexOf(fdl->decoders,cmd)==-1) {
			return hashint_table_add(fdl->decoders,cmd,function);

		}

	}

	return -1;
}


int fdl_unregister_function(framedecoder_list_t *fdl,int cmd) {

	if (fdl) {

		return hashint_table_delete(fdl->decoders,cmd,false);

	}

	return -1;
}

framedecoder_function_t fdl_get_function(framedecoder_list_t *fdl,int cmd) {

		if (fdl) {
			return (framedecoder_function_t) hashint_table_get(fdl->decoders,cmd);
		}

		return NULL;
}
