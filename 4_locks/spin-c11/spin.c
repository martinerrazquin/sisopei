#define _POSIX_C_SOURCE 200809


#include <stdio.h>
#include <pthread.h>
#include "lock.h"

static unsigned counter;
static lock11_t lock; //struct lock

static void *work(void *arg) {
    for (int i = 0; i < 1e7; i++) {
        acquire(&lock);
        counter++;
        release(&lock);
    }
    printf("%s done\n", (char *) arg);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    init(&lock);

    pthread_create(&t1, NULL, work, "A");
    pthread_create(&t2, NULL, work, "B");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("main done (counter = %u)\n", counter);
	return 0;
}

