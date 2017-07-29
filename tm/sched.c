#include <lib/list.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/sched.h>

struct runqueue rq;

void sched_init(void)
{
    memset(&rq, 0, sizeof(struct runqueue));
}

void runqueue_insert(struct runqueue* rq, struct thread* thread)
{
    LIST_APPEND(rq->runnable, thread);
    rq->num_threads++;
}

void runqueue_remove(struct runqueue* rq, struct thread* thread)
{
    LIST_REMOVE(rq->runnable, thread);
    rq->num_threads--;
}

struct thread* runqueue_next(struct runqueue* rq)
{
    if (!rq->current) {
        rq->current = rq->runnable;
        return rq->current;
    }
    if (!rq->current->next)
        return rq->current;
    rq->current = rq->current->next;
    return rq->current;
}
