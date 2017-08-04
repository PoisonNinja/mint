#include <kernel.h>
#include <mm/heap.h>
#include <mm/virtual.h>
#include <string.h>
#include <tm/sched.h>

static struct runqueue rq;

extern void process_init(void);

extern struct process* kernel_process;

void sched_init(void)
{
    process_init();
    memset(&rq, 0, sizeof(struct runqueue));
    list_runtime_init(&rq.runnable);
    struct thread* kinit = kzalloc(sizeof(struct thread));
    process_add_thread(kernel_process, kinit);
    sched_add(kinit);
    current_thread = kinit;
}

void sched_add(struct thread* thread)
{
    runqueue_insert(&rq, thread);
}

void sched_remove(struct thread* thread)
{
    if (thread == current_thread)
        current_thread = NULL;
    runqueue_remove(&rq, thread);
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
    if (thread == rq->current)
        rq->current = NULL;
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
