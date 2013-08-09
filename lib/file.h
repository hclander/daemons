#ifndef FILE_H_
#define FILE_H_

#include <stdio.h>

extern int fwritePadding(const char *filename, void *buf , size_t buffsize, size_t padsize);
extern int fwritePaddingEx(const char *filename, void *buf , size_t buffsize, size_t padsize,int pad);
extern int fwriteOffset(const char *filename, void *buf, size_t buffsize, long offset);

#endif /* FILE_H_ */
