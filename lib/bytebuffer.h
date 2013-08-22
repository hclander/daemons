/*
 * bytebuffer.h
 *
 *  Created on: 20/08/2013
 *      Author: jcmendez
 */

#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#define T bytebuffer_t

typedef struct T *T;

#define BB_UNSET_MARK -1
#define BB_ORDER_BIG_ENDIAN    0
#define BB_ORDER_LITTLE_ENDIAN 1

T    bb_create();

void bb_destroy(T *bb);

int  bb_getCapacity(T bb);
int  bb_getLimit(T bb);
T	 bb_setLimit(T bb, int limit);

T    bb_clear(T bb);
T    bb_flip(T bb);

int  bb_hasRemaining(T bb);
int  bb_isBufferOwn(T bb);
int  bb_isReadOnly(T bb);




int  bb_getPosition(T bb);
T	 bb_setPosition(T bb, int newPosition);

int  bb_remaining(T bb);

T	 bb_mark(T bb);
T	 bb_reset(T bb);
T	 bb_rewind	(T bb);

T	 bb_allocate(T bb, int capacity);

T    bb_wrap(T bb, void *src, int size);
T    bb_wrapOffset(T bb, void *src, int offset, int size);

int	bb_getOrder(T bb);
T	bb_setOrder(T bb,int order);

T	 bb_compact(T bb);

int  bb_compare(T bb, T bbDst);
int  bb_equals(T bb, T bbDst);

T	 bb_duplicate(T bb);

T	bb_slice(T bb);

void *bb_getBuffer(T bb);


int  bb_get(T bb);

T    bb_getToBuff(T bb, void *dst,int dstOffset, int dstCount);

int  bb_getByIdx(T bb, int idx);

int	 bb_getInt8(T bb);
int  bb_getInt16(T bb);
int  bb_getInt32(T bb);
long bb_getInt64(T bb);

int	 bb_getInt8Idx(T bb, int idx);
int  bb_getInt16Idx(T bb, int idx);
int  bb_getInt32Idx(T bb, int idx);
long bb_getInt64Idx(T bb, int idx);

//wchar_t bb_getWChar();


double bb_getDouble(T bb);
double bb_getDoubleByIdx(T bb, int idx);

float  bb_getFloat(T bb);
double bb_getFloatByIdx(T bb, int idx);

T	bb_put(T bb, int byte);
T	bb_putByIdx(T bb, int idx, int byte);
T	bb_putBuff(T bb, void *dst, int dstOffset, int dstCount);
T	bb_putByteBuffer(T bb, T bbSrc);   // Puts the remains bytes in bbSrc  into bb

T	bb_putInt8(T bb, int value);
T	bb_putInt16(T bb, int value);
T	bb_putInt32(T bb, int value);
T	bb_putInt64(T bb, long value);

T	bb_putInt8Idx(T bb, int idx, int value);
T	bb_putInt16Idx(T bb, int idx, int value);
T	bb_putInt32Idx(T bb, int idx, int value);
T	bb_putInt64Idx(T bb, int idx, long value);

T	bb_putDouble(T bb, double value);
T	bb_putDoubleIdx(T bb, int idx, double value);
T	bb_putFloat(T bb, float value);
T	bb_putFloatIdx(T bb, int idx, float value);


#undef T

#endif /* bb_H_ */
