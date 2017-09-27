#pragma once

#include <types.h>

struct block_device;
struct inode;

struct block_operations {
    ssize_t (*read)(struct block_device *, uint8_t *, size_t, off_t);
    ssize_t (*write)(struct block_device *, uint8_t *, size_t, off_t);
};

struct block_device {
    void *b_data;
    struct block_operations *b_ops;
};

extern int block_mount_inode(struct inode *inode, struct block_device *block);
