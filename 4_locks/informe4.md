# INFORME ENTREGA INDIVIDUAL 4: LOCKS


*Nota: a lo largo de la entrega hago referencias a "evitar competencias" por un recurso, me refiero a más de un thread realizando acciones luego de las cuales uno solo "ganará" y para el resto habrán sido tiempo de ejecución malgastado.

## spin-time
 

1.**Mostrar los resultados de la ejecución.**  
	real 1.99  
	user 3.92  
	sys 0.00
2.**Explicar qué significa cada uno de los tiempos (real, user y sys).**
	User es el tiempo durante el cual el procesador está ejecutando código a nivel usuario.  
	Sys es la contraparte de user a nivel kernel (syscalls).  
	Real es el tiempo desde que se inicia la ejecución del código hasta que termina, incluyendo tiempo en el cual NO se está ejecutando el código a medir.
3.**Mostrar los tiempos si se usan tres hilos en lugar de dos, y explicar los cambios observados.**
	real 3.80  
	user 10.80  
	sys 0.00  
	Al correr con 3 threads resulta que gastan mucho más tiempo "spinning"/haciendo nada (pero que es una operación) que incrementando el contador, y ahora hay 3 compitiendo por 1 solo recurso.

4.**Explicar por qué sys siempre es 0 con este programa.**
	Porque no se hacen llamadas al kernel, todo el código se ejecuta en ring 3.

## spin-xchg

*Nota: estos tiempos fueron para 2 threads*
### Tiempos nuevos:
real 3.27
user 6.53
sys 0.00


### spin-86.c
~~~
void init(struct lock *lk) {
	clear(&(lk->flag));
}
void acquire(struct lock *lk) {
	while(xchg(&(lk->flag),1));	//si habia un 1 da true
}
void release(struct lock *lk) {
	clear(&(lk->flag));
}
~~~

## spin-c11

### Tiempos nuevos:
real 2.35
user 4.70
sys 0.00

### spin-c11.c
~~~
#include <stdatomic.h>

struct lock {
    atomic_flag flag;
};

void init(struct lock *lk){
	atomic_flag_clear(&(lk->flag));
}
void acquire(struct lock *lk){
	while(atomic_flag_test_and_set(&(lk->flag)));
}
void release(struct lock *lk){
	atomic_flag_clear(&(lk->flag));
}
~~~

## 	sys-clone

### Tiempos nuevos:
real 2.10
user 4.19
sys 0.00

### spin-clone.c
~~~
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
~~~

*Asumiendo que se llamó a clone() sin el flag CLONE_THREAD: ¿qué se haría diferente, o no funcionaría, de haberlo usado?*
El wait no funcionaría porque el signal es enviado al parent process, y con CLONE_THREAD el thread creado tiene el mismo PPID que el creador.


## queue-spin

Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 30.20 | 16.20| 11.30 | 9.91
 U | 0.20 	| 0.99 | 2.29  | 4.99
 S | 0.00 	| 0.00 | 0.00  | 0.00
 C | 32/2 	| 32/6 | 32/51 | 34/91

## queue-mutex

Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 30.20 | 16.20| 11.30 | 9.90
 U | 0.20 	| 0.76 | 1.20  | 2.44
 S | 0.00 	| 0.22 | 1.05  | 2.40
 C | 32/3 	| 98/4 | 16131/38 | 90009/43

*Explicar las diferencias en tiempos y cambios de contexto respecto a la implementación con spin lock.*
Conforme aumenta la cantidad de threads se nota más como si bien el total de ejecución (U+S) casi no cambia, en el spin es todo tiempo a nivel de user y en mutex se distribuye con tiempo a nivel kernel. Esto se debe a que la implementación de spinlocks involucra switches involuntarios (sacar de ejecución el thread no lo hace el mismo código sino el scheduler) mientras que la implementación de los mutex se comporta como un spinlock durante un corto tiempo (de allí el tiempo considerable de S) y luego sí "se van a dormir". Lo antes explicado aplica también a la disparidad en C, donde los mutex realizan gran cantidad de context switch voluntarios mientras que los spinlock implican una cantidad mayor de involuntarios.

## queue-respin-wait

Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 31.00 | 17.00| 12.00 | 10.00
 U | 0.00 	| 0.00 | 0.00  | 0.00
 S | 0.00 	| 0.00 | 0.00  | 0.00
 C | 33/1 	| 35/1 | 38/1 | 40/3

*Comparar los tiempos reales de ejecución con los de CPU, en esta y versiones anteriores.*
Los tiempos reales de ejecución aumentaron, lo cual era esperable, pero mínimamente. Notar que para 4 threads la diferencia ya es de .1 seg, lo cual si además se tiene en cuenta que los tiempos de ejecución de código (U, S ya era nulo) resultan despreciables indica que la mayoría del tiempo se estaba esperando el lock.

## sys-futex

### Despertando de a 1
Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 30.20 |16.21 | 11.30 | 9.91
 U | 0.07 	| 0.38 | 0.74  | 1.48
 S | 0.12 	| 0.62 | 1.48  | 3.35
 C |  32/5	| 617/12 | 26469/70 | 111692/274

### Despertando a todos
Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 30.20 | 16.20| 11.30 | 9.90
 U | 0.04 	| 0.28 | 0.74  | 1.12
 S | 0.15 	| 0.69 | 1.52  | 3.69
 C | 32/3	| 323/10 | 28090/40 | 113532/384

### futex_lock.c
~~~
#include "lock.h"
#include "x86.h"
#include <linux/futex.h>
#include <sys/time.h>
#define _GNU_SOURCE 
#include <unistd.h>
#include <sys/syscall.h>
#include <limits.h> //INT_MAX

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
~~~

*Razonar las diferencias entre ambas versiones, y cómo comparan a la implementación de mutex de pthreads.*
La diferencia radica en que cuando se hace un unlock, uno despierta a 1 que estuviera esperando y el otro despierta a todos. Se esperaría que despertando de a 1 se logre un desempeño mejor al evitar las competencias por el lock, pero esto no ocurre. Una razón que se me ocurrió era que al haber 1 producer y 2+ consumers en promedio estuvieran pasándose entre consumers mucho tiempo el lock sin poder hacer nada hasta que por fin le llega al producer, y por eso no hay diferencia de tiempo, pero probando aparte noté que tampoco para 2 o 3 producers hay diferencia notoria entre despertar a 1 o a todos.
Sí se nota diferencia respecto de los mutex de pthreads, ya que los futex se parecen más a la idea teórica que se tiene de un mutex. De hecho, se podría decir que si de un lado se tiene los spinlocks y del otro esta implementación de locks con futex, los mutex de pthreads están a medio camino entre ambos. Un futex no pierde tiempo entre intentar tomar el lock e irse a dormir si no lo consigue, minimizando el tiempo en el que el thread está despierto mientras no tenga el lock.

## queue-condvar

Elem | 1 Consumidor | 2 Consumidores | 3 Consumidores | 4 Consumidores
-- | -- | -- | -- | --
 R | 30.20 			| 16.20			| 11.30 			| 9.90
 U | 0.00 			| 0.00 			| 0.00  			| 0.00
 S | 0.00 			| 0.00 			| 0.00				| 0.00
 C | 33/3 			| 35/1 			| 41/6 				| 47/1

### consumers-condvar.c
~~~
...
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER; 
...
static void *producer(__attribute__((unused)) void *arg) {
    unsigned seed = 17;
    struct timespec ts = {};
    for (int i = 0; i < ITEMS; i++) {
        ts.tv_nsec = 1e8 + (rand_r(&seed) % 9) * 1e8;
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&lock);
        available++;
		pthread_cond_broadcast(&cond);//si no es broadcast puede que algun thread quede dormido para siempre
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
...
static void *consumer(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (item == ITEMS) {
            pthread_mutex_unlock(&lock);
            break;  // All items consumed.
        }
        while (!available) {
			pthread_cond_wait(&cond,&lock);//retorna con lock lockeado
			if (item == ITEMS) { //hay que hacer doble chequeo porque el lock es normal y no errorchecking
            	pthread_mutex_unlock(&lock);
            	return NULL;  // All items consumed.
        	}        
		}
        printf("%s: consumed item %u\n", (char *) arg, ++item);
        available--;
        pthread_mutex_unlock(&lock);
        sleep(2);
    }
    return NULL;
}
~~~
*Explicar las diferencias tanto en tiempo como cambios de contexto.*
En tiempo real no hay mayor diferencia respecto de las versiones de locks, pero esto es debido a que la gran mayoría del tiempo los threads están durmiendo entre iteraciones de sus ciclos. Esto queda evidenciado en los tiempos U y S, que para la implementación con cond.vars es despreciable otra vez. La diferencia sustancial es que en respin-wait era porque se daba un "changüí" esperando que hubiera producción en el medio, ahora literalmente hasta que no se produzca no van a ser despertados. Obviamente al no ser despertados entre consumers, los cambios de contexto voluntarios (por irse a dormir) son llevados al mínimo (salvo por algún spurious wake-up), y los involuntarios se deben a los del scheduler.

*¿Qué dos versiones tardan menos tiempo real, y por qué? ¿Cuál tarda más, y por qué?*
La que más tarda es, lógicamente, la de respin-wait debido a los tiempos excesivos durante los que duerme, a la espera de que al despertar haya algo producido.
Para las que menos tiempo real usan es dificil decidir porque son todos tiempos similares (+- .01 seg). Es seguro que una de ellas es la implementación con cond. vars porque fueron pensadas para este tipo de problemas, y la segunda podría ser spinlocks o mutexes de pthreads, es complicado decidir porque para un tiempo corto de espera ambos se comportan igual. Asumo que en el caso general los mutexes tienen un mejor desempeño que los spinlocks.

## cond-unlock
*¿Por qué toma pthread_cond_wait() un mutex como segundo parámetro? Describir qué ocurriría si el consumidor usara la supuesta secuencia:*
~~~
... {
  pthread_mutex_unlock(&lock);
  pthread_cond_wait(&cond);
}
~~~
Primero, si hace un unlock luego de cond_wait debería hacer un lock, con lo que queda dividida la región crítica en 2 porque sí. Dicho esto, el problema está en el orden y falta de atomicidad de dichas 3 acciones. Se puede observar qué podría pasar entre que se pierde posesión del lock y se espera a la señal de seguir: al perder el lock, lo puede tomar el thread productor: hacer sus tareas, enviar la señal de continuación y volver. Suponiendo que ahora vuelve al consumidor, este se va a dormir esperando una señal que ya fue enviada, pudiendo no ser despertado jamás.
Es importante destacar que la atomicidad de todo este proceso lo que garantiza es que entre el chequeo de una condición no cumplida y el cond_wait, la señal a la que se está esperando NO será enviada, porque esto requiere el lock que posee actualmente el thread "que espera".
