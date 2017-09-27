#include <block/block.h>
#include <fs/device.h>
#include <fs/fs.h>
#include <mm/heap.h>

int block_mount_inode(struct inode* inode, struct block_device* block)
{
    struct fs_device* device = kmalloc(sizeof(struct fs_device));
    if (!device) {
        return 1;
    }
    device->block = block;
    inode->i_fs_dev = device;
    return 0;
}
