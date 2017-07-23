#include <fs/fs.h>
#include <fs/mount.h>
#include <fs/open.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <string.h>

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

struct filesystem* filesystem_get(const char* name)
{
    struct filesystem* fs;
    LIST_FOR_EACH(fs_list, fs)
    {
        if (!strcmp(fs->name, name))
            return fs;
    }
    return NULL;
}

extern void dentry_init(void);
extern void inode_init(void);
extern void file_init(void);
static int filesystem_init(void)
{
    // Initialize the inode allocator
    inode_init();
    // Initialize the dentry allocator
    dentry_init();
    // Initialize the file allocator
    file_init();
    return 0;
}
SUBSYS_INITCALL(filesystem_init);

extern struct inode* fs_root;

static int rootfs_init(void)
{
    struct filesystem* fs = filesystem_get("initfs");
    if (!fs) {
        printk(WARNING, "WTF is this, no initfs?\n");
        return 1;
    }
    struct superblock* sb = kmalloc(sizeof(struct superblock));
    int r = 0;
    if ((r = fs->mount(sb)) < 0) {
        kfree(sb);
        return r;
    }
    fs_root = sb->s_root;
    return 0;
}
LATE_INITCALL(rootfs_init);
