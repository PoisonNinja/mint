#include <fs/fs.h>
#include <fs/path.h>
#include <kernel.h>
#include <kernel/init.h>
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

extern void dentry_init(void);
extern void inode_init(void);
static int filesystem_init(void)
{
    // Initialize the inode allocator
    inode_init();
    // Initialize the dentry allocator
    dentry_init();
    path_resolve("/foo/bar/baz/cat", 0);
    return 0;
}
SUBSYS_INITCALL(filesystem_init);
