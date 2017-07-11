void arch_cpu_halt(void)
{
    __asm__ __volatile__("hlt");
}
