#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/stacktrace.h>
#include <kernel/version.h>

static char mint_banner[] =
    "Mint version " UTS_RELEASE " (" MINT_COMPILE_BY "@" MINT_COMPILE_HOST
    ") (" MINT_COMPILER ") " UTS_VERSION;

extern void setup_arch(void);

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", mint_banner);
    printk(INFO, "%llu KiB of memory available\n", bootinfo->total_mem);
    do_initcall(EARLY_INIT);
    do_initcall(CORE_INIT);
    setup_arch();
    do_initcall(ARCH_INIT);
    do_initcall(SUBSYS_INIT);
    do_initcall(FS_INIT);
    do_initcall(DEVICE_INIT);
    do_initcall(LATE_INIT);
    stacktrace();
    interrupt_enable();
    for (;;)
        ;
}
