#pragma once

#include <tm/process.h>
#include <tm/thread.h>

struct runqueue {
    struct thread* current_thread;
    struct thread* thread_queue;
};

extern struct runqueue* runqueue_create(void);
void runqueue_insert(struct runqueue* queue, struct thread* thread);
void runqueue_remove(struct runqueue* queue, struct thread* thread);
