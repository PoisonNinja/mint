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

#include <mm/heap.h>
#include <tm/process.h>
#include <tm/sched.h>
#include <tm/thread.h>

static tid_t next_tid = 0;

struct thread* current_thread;
extern struct process* kernel_process;

extern void arch_thread_set_stack(addr_t stack);

tid_t thread_get_available_tid(void)
{
    return next_tid++;
}

struct thread* thread_allocate(void)
{
    struct thread* thread = kzalloc(sizeof(struct thread));
    if (thread)
        thread->tid = thread_get_available_tid();
    return thread;
}

void thread_set_stack(addr_t stack)
{
    return arch_thread_set_stack(stack);
}

extern void arch_thread_switch(struct interrupt_ctx* ctx,
                               struct thread* current, struct thread* next);

void thread_switch(struct interrupt_ctx* ctx, struct thread* current,
                   struct thread* next)
{
    current_thread = next;
    thread_set_stack(next->kernel_stack);
    return arch_thread_switch(ctx, current, next);
}

void __attribute__((noreturn)) kthread_exit(void)
{
    struct thread* kthread = current_thread;
    sched_remove(kthread);
    process_remove_thread(kernel_process, kthread);
    // Loop until we're descheduled
    for (;;)
        __asm__("hlt");
}

extern void arch_kthread_setup_registers(struct thread* kthread,
                                         void (*fn)(void* data), void* arg,
                                         addr_t stack, size_t stack_size);

struct thread* kthread_create(void (*fn)(void* data), void* data)
{
    struct thread* kthread = thread_allocate();
    process_add_thread(kernel_process, kthread);
    addr_t stack = (addr_t)kmalloc(0x1000);
    arch_kthread_setup_registers(kthread, fn, data, stack, 0x1000);
    return kthread;
}
