#include "lock.h"
#include "x86.h"

void init(struct lock *lk) {
	clear(&(lk->flag));
}

void acquire(struct lock *lk) {
	while(xchg(&(lk->flag),1));	//si habia un 1 da true
}


void release(struct lock *lk) {
	clear(&(lk->flag));
}

