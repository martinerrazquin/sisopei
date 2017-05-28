#define _POSIX_C_SOURCE 200809

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include "lock.h"


#define FLAGS	CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM | SIGCHLD	

static unsigned counter;
static lock11_t lock; //struct lock

//sys-clone
#define STACK_SIZE (1 << 12)  // 4 KiB per thread.
#define NUM_THREADS 2
static char stack_space[NUM_THREADS][STACK_SIZE];

static int work(void *arg) {
    for (int i = 0; i < 1e7; i++) {
        acquire(&lock);
        counter++;
        release(&lock);
    }
    printf("%s done\n", (char *) arg);
    return 0;
}

int main(void) {
    pid_t t1, t2;
    init(&lock);
	t1=clone(work,(char*)stack_space+STACK_SIZE-1, FLAGS ,"A");	
	t2=clone(work,(char*)stack_space+2*STACK_SIZE-1, FLAGS,"B");

	if(t1==-1 || t2==-1) {
		fprintf(stderr,"Thread creation failed");
		return 1;
	}	
    waitpid(t1, NULL, 0);
    waitpid(t2, NULL, 0);

    printf("main done (counter = %u)\n", counter);
	return 0;
}

