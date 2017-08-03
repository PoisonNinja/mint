#pragma once

#include <fs/fs.h>
#include <lib/list.h>
#include <mm/virtual.h>
#include <tm/thread.h>
#include <types.h>

#define current_process ((current_thread)->process)

#define MAX_FD 128

struct files_table {
    struct file **files;
    uint8_t *file_bitset;
};

struct process {
    pid_t pid;
    uint32_t status;
    uint32_t flags;
    struct memory_context ctx;
    // Filesystem accounting
    struct files_table files;
    struct inode *root, *cwd;
    // Process tree stuff
    struct process *parent, *children;
    // Threading
    struct list_element threads;
};

extern struct process *process_allocate(void);
extern void process_add(struct process *process);
