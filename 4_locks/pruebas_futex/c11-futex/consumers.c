#define _POSIX_C_SOURCE 200809

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>  // rand_r
#include <time.h>    // nanosleep
#include <unistd.h>  // sleep
#include "lock.h"

#ifndef ITEMS
#define ITEMS 15
#endif

#ifndef CONSUMERS
#define CONSUMERS 2
#endif

static unsigned item;
static unsigned available;
static lock_futex11_t lock;

static void *producer(__attribute__((unused)) void *arg) {
    unsigned seed = 17;
    struct timespec ts = {};

    for (int i = 0; i < ITEMS; i++) {
        // Simulate work for producing an item.
        ts.tv_nsec = 1e8 + (rand_r(&seed) % 9) * 1e8;
        nanosleep(&ts, NULL);

        // Produce next item, marking it as available.
        acquire(&lock);
        available++;
        release(&lock);
    }
    return NULL;
}

static void *consumer(void *arg) {
    while (1) {
        acquire(&lock);

        if (item == ITEMS) {
            release(&lock);
            break;  // All items consumed.
        }

        if (!available) {
            release(&lock);
#ifdef RESPIN_WAIT
            sleep(1);
#endif
            continue;
        }

        // Simulate getting the item
        printf("%s: consumed item %u\n", (char *) arg, ++item);
        available--;

        // Release the lock and simulate some work.
        release(&lock);
        sleep(2);
    }
    return NULL;
}

int main(void) {
    pthread_t prod, consumers[CONSUMERS];
    char *names[] = {"A", "B", "C", "D", "E",
                     "F", "G", "H", "I", "J"};

    init(&lock);

    for (int i = 0; i < CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, names[i]);
    }

    pthread_create(&prod, NULL, producer, "Producer");

    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
}
