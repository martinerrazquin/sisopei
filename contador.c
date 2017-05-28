#include "funcs.h"

#define COUNTLEN 40
#define TICKS (1ULL << 20)
#define DELAY(x) (TICKS << (x))
#define CANT_ITER 50

void contador(unsigned char linea, char color, unsigned char delay) {
    char counter[COUNTLEN] = {'0'};  // Our ASCII digit counter (RTL).
	
	unsigned int iteraciones = CANT_ITER;
    while (iteraciones--) {
        char *buf = (void *) 0xb8000 + linea * 160;
        char *c = &counter[COUNTLEN];

        unsigned p = 0;
        unsigned long long i = 0;
/*
        while (i++ < DELAY(delay))
            sched();
*/
        while (counter[p] == '9') {
            counter[p++] = '0';
        }

        if (!counter[p]++) {
            counter[p] = '1';
        }

        while (c-- > counter) {
            *buf++ = *c;
            *buf++ = color;
        }

		while (i++ < DELAY(delay)){
            if (i % TICKS == 0) sched();
		}
    }
	
	//task_exit();
}
