#ifndef FUNCS_H
#define FUNCS_H

// Asigna a main() el índice 0, y lo marca como RUNNING. main()
// se convierte después en la “idle task” del sistema.
void task_init(void);

// Crea una nueva tarea, marcándola como READY. No se ejecutará hasta que se
// llame a sched().
void task_spawn(void (*entry)(void));

// Finaliza la tarea actual; no volverá a ejecutarse.
void task_exit(void);

// Planificador round-robin.
void sched(void);

// Imprime en una línea VGA un contador que se auto-incrementa. Para marcar su
// velocidad, hace (TICKS << delay) iteraciones por incremento. En cada TICKS,
// llama al planificador.
void contador(unsigned char linea, char color, unsigned char delay);

// Utilidades.
void *stack_alloc(int size);

#endif
