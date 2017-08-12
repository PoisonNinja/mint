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

#include <arch/mm/mm.h>
#include <kernel.h>
#include <tm/thread.h>

extern void tss_set_stack(addr_t stack);

void arch_thread_set_stack(addr_t stack)
{
    tss_set_stack(stack);
}

void arch_save_context(struct interrupt_ctx* ctx, struct thread* thread)
{
    struct registers* registers = &thread->registers;
    registers->rax = ctx->rax;
    registers->rbx = ctx->rbx;
    registers->rcx = ctx->rcx;
    registers->rdx = ctx->rdx;
    registers->rdi = ctx->rdi;
    registers->rsi = ctx->rsi;
    registers->rsp = ctx->rsp;
    registers->rbp = ctx->rbp;
    registers->r8 = ctx->r8;
    registers->r9 = ctx->r9;
    registers->r10 = ctx->r10;
    registers->r11 = ctx->r11;
    registers->r12 = ctx->r12;
    registers->r13 = ctx->r13;
    registers->rip = ctx->rip;
}

void arch_load_context(struct interrupt_ctx* ctx, struct thread* thread)
{
    struct registers* registers = &thread->registers;
    ctx->rax = registers->rax;
    ctx->rbx = registers->rbx;
    ctx->rcx = registers->rcx;
    ctx->rdx = registers->rdx;
    ctx->rdi = registers->rdi;
    ctx->rsi = registers->rsi;
    ctx->rsp = registers->rsp;
    ctx->rbp = registers->rbp;
    ctx->r8 = registers->r8;
    ctx->r9 = registers->r9;
    ctx->r10 = registers->r10;
    ctx->r11 = registers->r11;
    ctx->r12 = registers->r12;
    ctx->r13 = registers->r13;
    ctx->rip = registers->rip;
}

void arch_thread_switch(struct interrupt_ctx* ctx, struct thread* current,
                        struct thread* next)
{
    if (current)
        arch_save_context(ctx, current);
    /*
     * Retrieve and set the next address space. This is safe as long nothing
     * is corrupted, because the kernel is mapped into every address space
     * anyways
     */
    addr_t pml4 = next->process->ctx.page_table;
    if (!pml4)
        return;
    write_cr3(pml4);
    arch_load_context(ctx, next);
}

void arch_kthread_setup_registers(struct thread* kthread,
                                  void (*fn)(void* data), void* arg,
                                  addr_t stack, size_t stack_size)
{
    addr_t* stack_array = (addr_t*)stack + stack_size;
    stack_array[-1] = (addr_t)&kthread_exit;
    kthread->registers.rip = (addr_t)fn;
    kthread->registers.rsp = (addr_t)(stack_array - 1);
    kthread->kernel_stack = stack + stack_size;
}
