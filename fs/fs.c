#include <fs/fs.h>
#include <fs/mount.h>
#include <fs/path.h>
#include <kernel.h>
#include <kernel/init.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/process.h>

static struct list_element fs_list = LIST_COMPILE_INIT(fs_list);

int filesystem_register(struct filesystem* fs)
{
    printk(INFO, "fs: Adding filesystem %s with data structure at %p\n",
           fs->name, fs);
    list_add(&fs_list, &fs->list);
    return 0;
}

int filesystem_unregister(struct filesystem* fs)
{
    list_delete(&fs->list);
    return 0;
}

struct filesystem* filesystem_get(const char* name)
{
    struct filesystem* fs = NULL;
    list_for_each(&fs_list, list, fs)
    {
        if (!strcmp(fs->name, name))
            return fs;
    }
    return NULL;
}

extern void dentry_init(void);
extern void inode_init(void);
extern void file_init(void);
void filesystem_init(void)
{
    // Initialize the inode allocator
    inode_init();
    // Initialize the dentry allocator
    dentry_init();
    // Initialize the file allocator
    file_init();
}

extern struct inode* fs_root;

void rootfs_init(void)
{
    struct filesystem* fs = filesystem_get("initfs");
    if (!fs) {
        printk(WARNING, "WTF is this, no initfs?\n");
        return;
    }
    struct superblock* sb = kmalloc(sizeof(struct superblock));
    int r = 0;
    if ((r = fs->mount(sb)) < 0) {
        kfree(sb);
        return;
    }
    current_process->root = current_process->cwd = sb->s_root;
    return;
}
