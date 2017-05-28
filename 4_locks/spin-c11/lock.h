#ifndef _LOCKH_
#define _LOCKH_

#include <stdatomic.h>

struct lock {
    atomic_flag flag;
};

typedef struct lock lock11_t;

void init(struct lock *lk);
void acquire(struct lock *lk);
void release(struct lock *lk);

#endif
