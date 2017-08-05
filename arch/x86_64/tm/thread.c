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
