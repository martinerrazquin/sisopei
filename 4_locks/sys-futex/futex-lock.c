#include "lock.h"
#include "x86.h"
#include <linux/futex.h>
#include <sys/time.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>
#include <limits.h> //INT_MAX

/*
int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout,   // or: uint32_t val2 
                	int *uaddr2, int val3);
DONDE:
		uaddr		es la int* addr del flag
	
		futex_op 	es la operacion a ejecutar sobre el flag OR'd con 0+ opciones, siendo posibles: 
						COMO OPERACION:
							FUTEX_WAIT:	chequea (*uaddr == val), si TRUE sigue durmiendo hasta un FUTEX_WAKE. Si FALSE, devuelve EAGAIN.
										Todo esto vale para TIMEOUT==NULL (blocking indefinido). uaddr2 y val3 son ignorados.
							FUTEX_WAKE: despierta hasta val futexes que esten en FUTEX_WAIT de uaddr, por lo gral val == 1 o INT_MAX.
										Timeout, uaddr2 y val3 son ignorados.
							FUTEX_WAKE_OP para cond variables

*/
static int futex_wait(volatile int *uaddr, int val){
	return syscall(SYS_futex,uaddr,FUTEX_WAIT_PRIVATE,val,0,0,0);
}

static int futex_wake(volatile int *uaddr, int count){
	return syscall(SYS_futex,uaddr,FUTEX_WAKE_PRIVATE,count,0,0,0);
}

void init(struct lock *lk) {
	clear(&(lk->flag));
}

void acquire(struct lock *lk) {
	while(xchg(&(lk->flag),1)){
		futex_wait(&lk->flag,1);
	}
}


void release(struct lock *lk) {
	clear(&(lk->flag));
	#ifndef WAKE_ALL
	futex_wake(&lk->flag,1);		
	#else
	futex_wake(&lk->flag,INT_MAX);
	#endif
}


