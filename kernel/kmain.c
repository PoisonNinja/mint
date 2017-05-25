#include <boot/bootinfo.h>
#include <kernel.h>

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", "Hello world!");
    for (;;)
        ;
}
