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
#include "lib/mem.h"
#include "framedecoder_list.h"


// Lo defino así porque seguramente necesitaré añadir mas campos especificos a la estructura framedecoder_list_t
// para su gestion u otras cosas...

struct framedecoder_list_t {

	 hashint_table_t decoders;

};


framedecoder_list_t fdl_create() {
	framedecoder_list_t fdl ;
	NEW(fdl);

	fdl->decoders = hashint_table_create(0);

	return fdl;
}


void fdl_destroy(framedecoder_list_t *fdl){
	if (fdl && *fdl){
		hashint_table_destroy(&(*fdl)->decoders,false);
		FREE(*fdl);
	}
}

//TODO Permitir registar varias funciones para un mismod cmd ??
int fdl_register_func(framedecoder_list_t fdl,int cmd,framedecoder_func_t function) {

	if (fdl) {

		if (hashint_table_indexOf(fdl->decoders,cmd)==-1) {
			return hashint_table_add(fdl->decoders,cmd,function);
		}
	}

	return -1;
}


int fdl_unregister_func(framedecoder_list_t fdl,int cmd) {

	if (fdl) {

		return hashint_table_delete(fdl->decoders,cmd,false);

	}

	return -1;
}

framedecoder_func_t fdl_get_func(framedecoder_list_t fdl,int cmd) {

		if (fdl) {
			return (framedecoder_func_t) hashint_table_get(fdl->decoders,cmd);
		}

		return NULL;
}

int fdl_get_count(framedecoder_list_t fdl) {
	if (fdl)
		return hashint_table_getLen(fdl->decoders);
	return -1;
}

void *fld_get_list(framedecoder_list_t fdl) {
	if (fdl)
		return fdl->decoders;

	return NULL;
}
