#ifndef __INC_ETERBASE_RANDOM_H__
#define __INC_ETERBASE_RANDOM_H__

extern void				srandom(unsigned int seed);	// Cynex: bionic ile ayni imza
extern long				random();	// Cynex: bionic ile ayni imza
extern float			frandom(float flLow, float flHigh);
extern LONG				random_range(LONG from, LONG to);

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
