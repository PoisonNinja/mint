#pragma once

#include <fs/fs.h>

struct file_operations {
    int (*read)(struct inode *, struct file *, char *, int);
    int (*write)(struct inode *, struct file *, const char *, int);
};

struct file {
    struct inode *f_inode;
    struct dentry *f_dentry;
    off_t f_off;
    struct file_operations *f_ops;
};
