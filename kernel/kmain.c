#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/stacktrace.h>
#include <kernel/version.h>

static char mint_banner[] =
    "Mint version " UTS_RELEASE " (" MINT_COMPILE_BY "@" MINT_COMPILE_HOST
    ") (" MINT_COMPILER ") " UTS_VERSION;

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", mint_banner);
    printk(INFO, "%llu KiB of memory available\n", bootinfo->total_mem);
    stacktrace();
    interrupt_enable();
    for (;;)
        ;
}
