#include <arch/cpu/registers.h>
#include <kernel.h>

void virtual_fault(struct registers* registers, addr_t address, uint8_t reason);

int arch_virtual_fault(struct registers* registers,
                       void* __attribute__((unused)) dev)
{
    addr_t address = 0;
    uint8_t reason = 0;
    __asm__ volatile("mov %%cr2, %0" : "=r"(address));
    reason = registers->err_code;
    virtual_fault(registers, address, reason);
    return 0;
}
