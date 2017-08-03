#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <tm/process.h>

static struct hashmap* process_hashmap = NULL;

void process_init(void)
{
    process_hashmap = hashmap_create(1000);
}

struct process* process_allocate(void)
{
    struct process* process = kzalloc(sizeof(struct process));
    if (!process)
        return NULL;
    list_runtime_init(&process->threads);
    return process;
}

void process_add(struct process* process)
{
    hashmap_set(process_hashmap, &process->pid, sizeof(process->pid), process);
}
