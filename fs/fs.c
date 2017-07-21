#include <fs/fs.h>
#include <kernel.h>
#include <lib/list.h>

static struct filesystem* fs_list = NULL;

int filesystem_register(struct filesystem* fs)
{
    printk(INFO, "fs: Adding filesystem %s with data structure at %p\n",
           fs->name, fs);
    LIST_PREPEND(fs_list, fs);
    return 0;
}

int filesystem_unregister(struct filesystem* fs)
{
    LIST_REMOVE(fs_list, fs);
    return 0;
}
