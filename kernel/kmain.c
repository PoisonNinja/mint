#include <boot/bootinfo.h>
#include <kernel.h>
#include <kernel/version.h>

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "Mint version %d.%d.%d %s (GCC version %s) %s\n",
           MINT_VERSION_MAJOR, MINT_VERSION_MINOR, MINT_VERSION_PATCH,
           MINT_CODENAME, MINT_COMPILER_VERSION, MINT_COMPILE_TIME);
    for (;;)
        ;
}
