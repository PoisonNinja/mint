#include <boot/bootinfo.h>
#include <kernel.h>
#include <kernel/version.h>

static char mint_banner[] =
    "Mint version " UTS_RELEASE " (" MINT_COMPILE_BY "@" MINT_COMPILE_HOST
    ") (" MINT_COMPILER ") " UTS_VERSION;

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", mint_banner);
    for (;;)
        ;
}