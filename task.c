#include "task.h"
#include "funcs.h"

#define MAX_TASK 128
#define STACK_SIZE 4096

static struct Task *current;
static struct Task Tasks[MAX_TASK];

extern void swtch(unsigned **oldsp, unsigned **newsp);

void task_init() {
    current = &Tasks[0];
    current->status = RUNNING;
}

void task_spawn(void (*entry)(void)) {
    unsigned i = 0;

    // Encontrar la siguiente posición libre.
    while (i < MAX_TASK && Tasks[i].status != FREE)
        i++;

    void *stack = stack_alloc(STACK_SIZE) - sizeof(void*);

	//agrego la retaddr  
	void (**exit)(void) = stack;
	*exit = &task_exit;
	
	//Hago lugar para el TaskData inicial
	stack-=sizeof(struct TaskData);
    Tasks[i].stack = stack;
    Tasks[i].status = READY;

    // Preparar el stack conforme a lo que espera swtch().
    struct TaskData *d = stack;
    *d = (const struct TaskData){};  // Inicializa a 0.
    d->entry_fn = (unsigned) entry;
}

void sched() {
    struct Task *new = 0;
    struct Task *old = current;
/*
    for (int i = 0; i < MAX_TASK; i++) {
        if (Tasks[i].status == READY)
            new = &Tasks[i];
    }
*/

	for (struct Task* i = current+1;i!=current;i++){
		if (i >= Tasks + MAX_TASK) i = Tasks;
		if (i->status == READY){
            new = i;
			break;
		}
	}
    if (new) {
        if (old->status == RUNNING) old->status = READY;  
        current = new;
        current->status = RUNNING;
        swtch(&old->stack, &current->stack);
    }
}

void task_exit(void){
	current->status = FREE;
	sched();
}
