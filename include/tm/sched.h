#pragma once

#include <lib/list.h>
#include <tm/process.h>
#include <tm/thread.h>

// A per-CPU struct to represent all runnable tasks
struct runqueue {
    struct thread* current;
    struct thread* idle;
    struct list_element runnable;
    size_t num_threads;
};

struct interrupt_ctx;

extern void sched_init(void);
extern void sched_add(struct thread* thread);
extern void sched_remove(struct thread* thread);
extern void sched_tick(struct interrupt_ctx* ctx);

extern void runqueue_insert(struct runqueue* rq, struct thread* thread);
extern void runqueue_remove(struct runqueue* rq, struct thread* thread);
extern struct thread* runqueue_next(struct runqueue* rq);
