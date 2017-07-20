#pragma once

#include <mm/mm.h>
#include <tm/thread.h>
#include <types.h>

struct process {
    pid_t pid;
    uint32_t status;
    uint32_t flags;
    struct memory_context ctx;
    // Process tree stuff
    struct process* parent;
    struct process* children;
    // Threading
    struct thread* threads;
};
