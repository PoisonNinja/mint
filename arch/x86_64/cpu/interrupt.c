#include <arch/cpu/registers.h>

static char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check?",
    "Machine Check!",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Unknown",
};

void arch_interrupt_disable(void)
{
    __asm__("cli");
}

void arch_interrupt_enable(void)
{
    __asm__("sti");
}

extern void exception_dispatch(struct registers*);
void x86_64_exception_handler(struct registers* regs)
{
    exception_dispatch(regs);
}

extern void interrupt_dispatch(struct registers*);
void x86_64_interrupt_handler(struct registers* regs)
{
    regs->int_no -= 32;  // Translate IRQ into 0 based
    interrupt_dispatch(regs);
}

char* arch_exception_translate(int exception_no)
{
    if (exception_no > 31)
        return exception_messages[32];
    else
        return exception_messages[exception_no];
}
