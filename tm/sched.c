#include <kernel.h>
#include <mm/heap.h>
#include <mm/virtual.h>
#include <string.h>
#include <tm/sched.h>

struct runqueue rq;

extern void process_init(void);

void sched_init(void)
{
    process_init();
    memset(&rq, 0, sizeof(struct runqueue));
    list_runtime_init(&rq.runnable);
    struct process* kernel_process = process_allocate();
    kernel_process->ctx.page_table = kernel_context.page_table;
    struct thread* kinit = kzalloc(sizeof(struct thread));
    struct thread* kidle = kzalloc(sizeof(struct thread));
    list_add(&kernel_process->threads, &kinit->process_list);
    kinit->process = kernel_process;
    process_add(kernel_process);
    sched_add(kinit);
    current_thread = kinit;
}

void sched_add(struct thread* thread)
{
    runqueue_insert(&rq, thread);
}

void sched_tick(struct interrupt_ctx* ctx)
{
    struct thread* next = runqueue_next(&rq);
    thread_switch(ctx, current_thread, next);
}

void runqueue_insert(struct runqueue* rq, struct thread* thread)
{
    list_add(&rq->runnable, &thread->runqueue_list);
    rq->num_threads++;
}

void runqueue_remove(struct runqueue* rq, struct thread* thread)
{
    if (rq->current == thread) {
        rq->current = NULL;
    }
    list_delete(&thread->runqueue_list);
    rq->num_threads--;
}

struct thread* runqueue_next(struct runqueue* rq)
{
    if (rq->current) {
        rq->current = list_next_entry(rq->current, runqueue_list);
        if (&rq->current->runqueue_list == &rq->runnable) {
            rq->current = list_next_entry(rq->current, runqueue_list);
        }
    } else {
        rq->current =
            list_first_entry(&rq->runnable, struct thread, runqueue_list);
    }
    return rq->current;
}
