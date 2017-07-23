#include <errno.h>
#include <fs/fs.h>
#include <fs/mount.h>
#include <fs/path.h>
#include <kernel.h>
#include <lib/list.h>
#include <mm/heap.h>

int mount_fs(const char* special, const char* mountpoint, const char* name,
             uint32_t flags)
{
    struct filesystem* fs = filesystem_get(name);
    if (!fs) {
        printk(WARNING, "Invalid filesystem requested: %s\n", name);
        return -ENODEV;
    }
    // Sadly, no slab_cache for you
    struct superblock* sb = kmalloc(sizeof(struct superblock));
    int r = 0;
    if ((r = fs->mount(sb)) < 0) {
        kfree(sb);
        return r;
    }
    struct mountpoint* mp = kmalloc(sizeof(struct mountpoint));
    mp->mp_sb = sb;
    mp->mp_inode = sb->s_root;
    struct inode* mountpoint_inode = path_resolve(mountpoint, 0, 0);
    if (!mountpoint_inode) {
        kfree(mp);
        kfree(sb);
        return -ENOENT;
    }
    LIST_PREPEND(mountpoint_inode->i_mp, mp);
    return 0;
}
