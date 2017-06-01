#include <kernel.h>
#include <types.h>

void arch_stacktrace(void)
{
    uint64_t* rbp = NULL;
    __asm__("mov %%rbp, %0" : "=r"(rbp));
    while (rbp) {
        uint64_t rip = rbp[1];
        if (!rip)
            break;
        rbp = (uint64_t*)rbp[0];
        printk(INFO, "<%llX> %s\n", rip, "???????????");
    }
}
