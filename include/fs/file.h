#pragma once

#include <fs/fs.h>

struct file_operations {
    ssize_t (*read)(struct file *, uint8_t *, size_t);
    ssize_t (*write)(struct file *, uint8_t *, size_t);
};

struct file {
    struct inode *f_inode;
    struct dentry *f_dentry;
    off_t f_off;
    struct file_operations *f_ops;
};

extern struct file *file_allocate(void);
extern void file_free(struct file *file);

extern ssize_t file_pread(struct file *file, uint8_t *buffer, size_t size);
extern ssize_t file_read(struct file *file, uint8_t *buffer, size_t size);
extern ssize_t file_pwrite(struct file *file, uint8_t *buffer, size_t size);
extern ssize_t file_write(struct file *file, uint8_t *buffer, size_t size);
extern struct file *file_open(const char *name, uint32_t flags, mode_t mode);
