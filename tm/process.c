/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
