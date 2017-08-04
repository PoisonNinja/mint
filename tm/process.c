#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <tm/process.h>

static struct hashmap* process_hashmap = NULL;
struct process* kernel_process = NULL;

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

void process_add_thread(struct process* process, struct thread* thread)
{
    thread->process = process;
    list_add(&process->threads, &thread->process_list);
}

void process_remove_thread(struct process* __attribute__((unused)) process,
                           struct thread* thread)
{
    thread->process = NULL;
    list_delete(&thread->process_list);
}

void process_init(void)
{
    process_hashmap = hashmap_create(1000);
    kernel_process = process_allocate();
    kernel_process->ctx.page_table = kernel_context.page_table;
    process_add(kernel_process);
}
