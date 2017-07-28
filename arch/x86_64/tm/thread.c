#include <arch/mm/mm.h>
#include <tm/thread.h>

extern void tss_set_stack(addr_t stack);

void arch_thread_set_stack(addr_t stack)
{
    tss_set_stack(stack);
}

void arch_thread_switch(struct thread* current, struct thread* next)
{
    /*
     * Retrieve and set the next address space. This is safe as long nothing
     * is corrupted, because the kernel is mapped into every address space
     * anyways
     */
    addr_t pml4 = next->process->ctx.page_table;
    if (!pml4)
        return;
    write_cr3(pml4);
}
