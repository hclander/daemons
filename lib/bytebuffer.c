/*
 * bytebuffer.c
 *
 *  Created on: 20/08/2013
 *      Author: jcmendez
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <endian.h>

#include "mem.h"
#include "bytebuffer.h"

struct bytebuffer_t {

	int capacity;
	int limit;
	int position;
	int offset;
	int mark;

	int order;  // Endianness

	int readOnly;

	int ownBuffer;

	unsigned char *buf;

};

#define T bytebuffer_t


T    bb_create(){

	bytebuffer_t bb;

	NEW0(bb);

	return bb;
}

void bb_destroy(T *bb) {
	if (bb || (*bb)) {

		if ((*bb)->ownBuffer) {
			free((*bb)->buf);
		}

		FREE(*bb);
	}
}

T    bb_init(T bb, int capacity,  int ownBuffer, int offset, void *buf) {

	bb->limit = bb->capacity = capacity;
	bb->position = 0;
	bb->mark = UNSET_MARK;
	bb->ownBuffer = ownBuffer;
	bb->offset = offset;
	bb->buf = buf;

	return bb;

}

int  bb_getCapacity(T bb) {
	return bb->capacity;
}

int  bb_getLimit(T bb){
	return bb->limit;
}

T	 bb_setLimit(T bb, int limit) {

	if (limit>bb->capacity)
		return NULL;

	bb->limit = limit;

	if (bb->position>limit)
		bb->position=limit;

	if ( (bb->mark != UNSET_MARK) && (bb->mark > limit))
		bb->mark = limit;

	return bb;
}

T    bb_clear(T bb){

	bb->position =0;
	bb->mark = UNSET_MARK;
	bb->limit = bb->capacity;
	return bb;
}

T    bb_flip(T bb){

	bb->limit = bb->position;
	bb->position = 0;
	bb->mark = UNSET_MARK;
	return bb;

}

int  bb_hasRemaining(T bb){

	return (bb->position<bb->limit);
}

int  bb_isBufferOwn(T bb) {
	return bb->ownBuffer;
}

int  bb_isReadOnly(T bb) {

	//TODO: De momento no se implementan buffers ReadOnly
	return false;
}

int  bb_getPosition(T bb) {
		return bb->position;
}

T	 bb_setPosition(T bb, int newPosition){

	if (newPosition>bb->limit)
		return NULL;

	bb->position = newPosition;

	return bb;
}



int  bb_remaining(T bb) {
	return bb->limit-bb->position;
}

T	 bb_mark(T bb) {
	bb->mark = bb->position;
	return bb;
}

T	 bb_reset(T bb) {
	if (bb->mark == UNSET_MARK)
		return NULL;

	bb->position = bb->mark;


	return bb;
}

T	 bb_rewind	(T bb) {
	bb->position = 0;
	bb->mark = UNSET_MARK;
	return bb;
}

T	 bb_allocate(T bb, int capacity) {

	if (!bb->ownBuffer)
		bb->buf = NULL;

	RESIZE(bb->buf,capacity);

	bb_init(bb,capacity,true,0,bb->buf);

	return bb;
}

T    bb_wrapOffset(T bb, void *src, int offset, int size) {

	if (bb->ownBuffer)
		free(bb->buf);

	return bb_init(bb,size,0,offset,src);

}

T    bb_wrap(T bb, void *src, int size) {

	return bb_wrapOffset(bb,src,0,size);
}


int	bb_getOrder(T bb) {
	return bb->order;
}

T	bb_setOrder(T bb,int order) {
	bb->order = order;
	return bb;
}

T	 bb_compact(T bb) {

	 memmove(bb->buf+bb->offset,bb->buf+(bb->offset+bb->position),bb_remaining(bb));
	 bb->position = bb->limit -bb->position;
	 bb->limit = bb->capacity;
	 bb->mark = UNSET_MARK;
	 return bb;
}

int  bb_compare(T bb, T bbDst) {

}

int  bb_equals(T bb, T bbDst) {

	int result = false;

	if (bb_remaining(bb) == bb_remaining(bbDst)) {

		int p = bb->position;
		int pDst = bbDst->position;

		result = true;

		while (result && p <bb->limit)
			result = bb_getByIdx(p++) == bb_getByIdx(pDst++);

	}

	return result;
}

T	 bb_duplicate(T bb) {

	bytebuffer_t nn;

	NEW0(nn);


	nn->capacity = bb->capacity;
	nn->position = bb->position;
	nn->limit    = bb->limit;
	nn->mark     = bb->mark;
	nn->readOnly = bb->readOnly;
	nn->ownBuffer = 0;       // En los duplicados consideramos que el buffer no es suyo. De ese modo se impide liberar la memoria varias veces
	nn->order = bb->order;
	nn->buf = bb->buf;

	return nn;

}

T	bb_slice(T bb) {
	bytebuffer_t slice;

	NEW0(slice);

	return bb_init(slice,bb_remaining(bb),0,bb->offset+bb->position,bb->buf);

}

void *bb_getBuffer(T bb) {
	return bb->buf;
}

