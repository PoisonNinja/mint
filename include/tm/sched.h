#pragma once

#include <tm/process.h>
#include <tm/thread.h>

// A per-CPU struct to represent all runnable tasks
struct runqueue {
    struct thread* current;
    struct thread* runnable;
    size_t num_threads;
};

extern void sched_init(void);

extern void runqueue_insert(struct runqueue* rq, struct thread* thread);
extern void runqueue_remove(struct runqueue* rq, struct thread* thread);
extern struct thread* runqueue_next(struct runqueue* rq);
