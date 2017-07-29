#include <mm/heap.h>
#include <string.h>
#include <tm/sched.h>

struct runqueue rq;

void sched_init(void)
{
    memset(&rq, 0, sizeof(struct runqueue));
    list_runtime_init(&rq.runnable);
}

void runqueue_insert(struct runqueue* rq, struct thread* thread)
{
    list_add(&rq->runnable, &thread->runqueue_list);
    rq->num_threads++;
}

void runqueue_remove(struct runqueue* rq, struct thread* thread)
{
    list_delete(&thread->runqueue_list);
    rq->num_threads--;
}

struct thread* runqueue_next(struct runqueue* rq)
{
    return NULL;
}
