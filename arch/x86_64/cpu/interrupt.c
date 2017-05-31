void arch_interrupt_disable(void)
{
    __asm__("cli");
}

void arch_interrupt_enable(void)
{
    __asm__("sti");
}
