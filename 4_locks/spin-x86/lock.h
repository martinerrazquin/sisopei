#ifndef _LOCKH_
#define _LOCKH_

struct lock {
    volatile int flag;
};

typedef struct lock lock86_t;

void init(struct lock *lk);
void acquire(struct lock *lk);
void release(struct lock *lk);


#endif
