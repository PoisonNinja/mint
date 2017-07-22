#pragma once

#include <fs/fs.h>
#include <mm/virtual.h>
#include <tm/thread.h>
#include <types.h>

struct process {
    pid_t pid;
    uint32_t status;
    uint32_t flags;
    struct memory_context ctx;
    struct inode *root, *cwd;
    // Process tree stuff
    struct process *parent, *children;
    // Threading
    struct thread *threads;
};

extern struct process *current_process;
