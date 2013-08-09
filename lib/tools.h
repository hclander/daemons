#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdlib.h>
#include <time.h>

#define EARTH_OFFSET 268435456
#define EARTH_RADIUS 85445659.4471
#define DEG_RAD (3.14159 / 180)

extern void rnd_init();
extern int rnd(int max);
extern int rnd0(int max);
extern int rnd1(int max);
extern int rndAB(int low, int max);


extern void freeAndNull(void **ptr);


long lon2x(float lon);
long lat2y(float lat);
float x2lon(long x);
float y2lat(long y);


#endif /* TOOLS_H_ */
