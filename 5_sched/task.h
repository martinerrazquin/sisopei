#ifndef TASK_H
#define TASK_H

enum TaskStatus {
    FREE = 0,
    READY,
    RUNNING,
};

struct Task {
    unsigned *stack;
    enum TaskStatus status;
};

struct TaskData {
    // Registers as pushed by pusha.
    unsigned reg_edi;
    unsigned reg_esi;
    unsigned __unused_ebp;
    unsigned __unused_esp;
    unsigned reg_ebx;
    unsigned reg_edx;
    unsigned reg_ecx;
    unsigned reg_eax;

    // Saved eflags.
    unsigned reg_eflags;

    // Saved %ebp; makes swtch’s code simpler.
    unsigned reg_ebp;

    // Return address used in swtch’s "ret".
    unsigned entry_fn;
} __attribute__((packed));

#endif
