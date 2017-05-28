#include "funcs.h"

static void contador1(void) { contador(0, 0x2f, 1); }  // Verde, rápido.
static void contador2(void) { contador(3, 0x6f, 5); }  // Naranja, lento.
static void contador3(void) { contador(7, 0x4f, 7); }  // Rojo, muy lento.

int main(void) {
    task_init();

    // Con esta línea, no habría concurrencia. Solo se ejecutaría
    // el primer contador.
    // contador1();

    task_spawn(contador1);
    task_spawn(contador2);
    task_spawn(contador3);

    while (1) {
        sched();  // Become the idle task.
    }
}

void *stack_alloc(int size) {
    static void *next = (void *) (8 << 20);  // 8 MiB, arbitrariamente.
    void *ret = next;
    next -= size;
    return ret;
}
