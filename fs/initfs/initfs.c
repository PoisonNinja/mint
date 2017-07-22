#include <fs/fs.h>
#include <kernel/init.h>

static int initfs_mount(struct superblock* sb)
{
}

static struct filesystem initfs_filesystem = {
    .name = "initfs", .mount = initfs_mount,
};

static int init_initfs(void)
{
    filesystem_register(&initfs_filesystem);
    return 0;
}
FS_INITCALL(init_initfs);
