#pragma once

#include <types.h>

#define DEV_BITS 20
#define DEV_MASK 0xFFF

#define MAJOR(dev) ((dev) >> DEV_BITS)
#define MINOR(dev) ((dev)&DEV_MASK)
#define MKDEV(major, minor) (((major) << DEV_BITS) | (minor))

struct block_device;

struct fs_device {
    union {
        struct block_device* block;
    };
};

struct inode;

extern struct inode* mknod(const char* path, mode_t mode, dev_t dev);
