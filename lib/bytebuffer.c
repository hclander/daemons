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


T  bb_create(){

	T bb;

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

T  bb_setLimit(T bb, int limit) {

	if (limit>bb->capacity)
		return NULL;

	bb->limit = limit;

	if (bb->position>limit)
		bb->position=limit;

	if ( (bb->mark != UNSET_MARK) && (bb->mark > limit))
		bb->mark = limit;

	return bb;
}

T bb_clear(T bb){

	bb->position =0;
	bb->mark = UNSET_MARK;
	bb->limit = bb->capacity;
	return bb;
}

T bb_flip(T bb){

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

T  bb_setPosition(T bb, int newPosition){

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

	// TODO implement this
	return 0;
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

	T nn;

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
	T slice;

	NEW0(slice);

	return bb_init(slice,bb_remaining(bb),0,bb->offset+bb->position,bb->buf);

}

void *bb_getBuffer(T bb) {
	return bb->buf;
}


int  bb_get(T bb) {

	 return bb_getInt8(bb);

}

int  bb_getByIdx(T bb, int idx) {
	 return bb_getByIdx(bb);
}

T   bb_getToBuff(T bb, void *dst,int dstOffset, int dstCount) {

	if (bb->offset+bb->position+dstCount>bb->limit)
		return NULL;

	//FIXME

	while(dstCount--)
		*(unsigned char *)(dst+dstOffset++)=bb->buf[bb->offset+bb->position++];

	return bb;
}



int	 bb_getInt8(T bb) {

	if ((bb->position+sizeof(uint8_t))>bb->limit)
		return -1;

	return bb->buf[bb->offset+bb->position++];

}

int  bb_getInt16(T bb) {

	int value;

	if (bb->position+sizeof(uint16_t)>bb->limit)
		return -1;


	value =*((uint16_t *)(bb->buf+bb->offset+bb->position));

	bb->position += sizeof(uint16_t);

	return (bb->order == BIG_ENDIAN) ? be16toh(value) : le16toh(value);


}

int  bb_getInt32(T bb) {

	int value;

	if (bb->position+sizeof(uint32_t)>bb->limit)
			return -1;

	value =*((uint32_t *)(bb->buf+bb->offset+bb->position));

	bb->position += sizeof(uint32_t);

	return (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);
}

long bb_getInt64(T bb) {
	long value;

	if (bb->position+sizeof(uint64_t)>bb->limit)
				return -1;

		value =*((uint64_t *)(bb->buf+bb->offset+bb->position));

		bb->position += sizeof(uint64_t);

		return (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);

}

int	 bb_getInt8Idx(T bb, int idx) {

	if ( (idx<0) || (idx+sizeof(uint8_t)>bb->limit))
			return -1;

	return bb->buf[bb->offset+idx];

}

int  bb_getInt16Idx(T bb, int idx){
	int value;

	if ((idx<0) || idx+sizeof(uint16_t)>bb->limit)
		return -1;


	value =*((uint16_t *)(bb->buf+bb->offset+idx));


	return (bb->order == BIG_ENDIAN) ? be16toh(value) : le16toh(value);
}

int  bb_getInt32Idx(T bb, int idx) {
	int value;

	if ((idx<0) || idx+sizeof(uint32_t)>bb->limit)
			return -1;

	value =*((uint32_t *)(bb->buf+bb->offset+idx));


	return (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);
}

long bb_getInt64Idx(T bb, int idx) {
	long value;

	if ((idx<0) || idx+sizeof(uint64_t)>bb->limit)
				return -1;

	value =*((uint64_t *)(bb->buf+bb->offset+idx));


	return (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);
}

//wchar_t bb_getWChar();

float  bb_getFloat(T bb) {
	int value;

	if (bb->position+sizeof(float)>bb->limit)
			return -1;

	value =*((uint32_t *)(bb->buf+bb->offset+bb->position));

	bb->position += sizeof(float);

	value = (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);

	return *((float *)&value);

}

double bb_getFloatByIdx(T bb, int idx) {

	int value;

	if ((idx<0) || idx+sizeof(float)>bb->limit)
			return -1;

	value =*((uint32_t *)(bb->buf+bb->offset+idx));


	value = (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);

	return *((float *)&value);

}



double bb_getDouble(T bb) {
	long value;

	if (bb->position+sizeof(uint64_t)>bb->limit)
			return -1;

	value =*((uint64_t *)(bb->buf+bb->offset+bb->position));

	bb->position += sizeof(uint64_t);

	value = (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);

	return  *((double *)&value);

}

double bb_getDoubleByIdx(T bb, int idx) {
	long value;

	if ((idx<0) || idx+sizeof(uint64_t)>bb->limit)
			return -1;

	value =*((uint64_t *)(bb->buf+bb->offset+idx));


	value = (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);

	return  *((double *)&value);

}

T	bb_put(T bb, int byte) {

	return bb_putInt8(bb, byte);

}
T	bb_putByIdx(T bb, int idx, int byte) {
	return bb_putInt8Idx(bb,idx,byte);
}


T	bb_putBuff(T bb, void *src, int srcOffset, int srcCount) {


	if (bb->offset+bb->position+srcCount>bb->limit)
		return NULL;

	//FIXME

	while(srcCount--)
		bb->buf[bb->offset+bb->position++] = *(unsigned char *)(src+srcOffset++);

	return bb;

}

T	bb_putByteBuffer(T bb, T bbSrc) {   // Puts the remains bytes in bbSrc  into bb

	 return bb_putBuff(bb,bbSrc->buf,bbSrc->offset+bbSrc->position,bb_remaining(bbSrc));
}


T	bb_putInt8(T bb, int value) {
	if ((bb->position+sizeof(uint8_t))>bb->limit)
			return NULL;

	bb->buf[bb->offset+bb->position++] = value;

	return bb;
}


T	bb_putInt16(T bb, int value) {


	if (bb->position+sizeof(uint16_t)>bb->limit)
		return NULL;

	*((uint16_t *)(bb->buf+bb->offset+bb->position)) = (bb->order == BIG_ENDIAN) ? be16toh(value) : le16toh(value);

	bb->position += sizeof(uint16_t);

	return bb;

}

T	bb_putInt32(T bb, int value) {

	if (bb->position+sizeof(uint32_t)>bb->limit)
			return NULL;

	*((uint32_t *)(bb->buf+bb->offset+bb->position)) = (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);

	bb->position += sizeof(uint32_t);

	return bb;

}


T	bb_putInt64(T bb, long value) {

	if (bb->position+sizeof(uint64_t)>bb->limit)
			return NULL;

	*((uint64_t *)(bb->buf+bb->offset+bb->position)) = (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);

	bb->position += sizeof(uint64_t);

	return bb;

}

T	bb_putInt8Idx(T bb, int idx, int value) {

	if ((idx<0) || (idx+sizeof(uint8_t))>bb->limit)
		return NULL;

	bb->buf[bb->offset+idx] = value;

	return bb;

}

T	bb_putInt16Idx(T bb, int idx, int value) {


	if ((idx>0) || idx+sizeof(uint16_t)>bb->limit)
		return NULL;


	*((uint16_t *)(bb->buf+bb->offset+idx)) = (bb->order == BIG_ENDIAN) ? be16toh(value) : le16toh(value);

	return bb;
}

T	bb_putInt32Idx(T bb, int idx, int value) {

	if ((idx<0) || idx+sizeof(uint32_t)>bb->limit)
		return NULL;


	*((uint32_t *)(bb->buf+bb->offset+idx)) = (bb->order == BIG_ENDIAN) ? be32toh(value) : le32toh(value);

	return bb;
}

T	bb_putInt64Idx(T bb, int idx, long value) {

	if ((idx<0) || idx+sizeof(uint64_t)>bb->limit)
		return NULL;


	*((uint64_t *)(bb->buf+bb->offset+idx)) = (bb->order == BIG_ENDIAN) ? be64toh(value) : le64toh(value);

	return bb;
}

T	bb_putDouble(T bb, double value) {

	if (bb->position+sizeof(float)>bb->limit)
				return NULL;

	// Esto se basa en que sizeof(double) == sizeof(uint64_t)

	*((uint64_t *)(bb->buf+bb->offset+bb->position)) = (bb->order == BIG_ENDIAN) ? be64toh(*((uint64_t *)&value)) : le64toh(*((uint64_t *)&value));

	bb->position += sizeof(double);

	return bb;

}

T	bb_putDoubleIdx(T bb, int idx, double value) {

	if ((idx<0) || idx+sizeof(float)>bb->limit)
					return NULL;

	*((uint64_t *)(bb->buf+bb->offset+idx)) = (bb->order == BIG_ENDIAN) ? be64toh(*((uint64_t *)&value)) : le64toh(*((uint64_t *)&value));


	return bb;
}

T	bb_putFloat(T bb, float value) {

	if (bb->position+sizeof(float)>bb->limit)
			return NULL;

	// Esto se basa en que sizeof(float) == sizeof(uint32_t)
	*((uint32_t *)(bb->buf+bb->offset+bb->position)) = (bb->order == BIG_ENDIAN) ? be32toh(*((uint32_t *)&value)) : le32toh(*((uint32_t *)&value));

	bb->position += sizeof(float);

	return bb;

}

T	bb_putFloatIdx(T bb, int idx, float value) {

	if ((idx<0) || idx+sizeof(float)>bb->limit)
			return NULL;

	*((uint32_t *)(bb->buf+bb->offset+idx)) = (bb->order == BIG_ENDIAN) ? be32toh(*((uint32_t *)&value)) : le32toh(*((uint32_t *)&value));

	return bb;

}


