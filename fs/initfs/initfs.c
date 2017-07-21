#include <fs/fs.h>
#include <kernel/init.h>

static struct filesystem initfs_filesystem = {
    .name = "initfs",
};

static int init_initfs(void)
{
    filesystem_register(&initfs_filesystem);
    return 0;
}
FS_INITCALL(init_initfs);
