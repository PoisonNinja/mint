#pragma once

#include <types.h>

#define DIRENT_NAME_MAX 256

struct inode;
struct dentry;
struct file;
struct superblock;
struct mountpoint;

struct mountpoint {
    struct mountpoint *next, *prev;
};

struct inode_operations {
    int (*create)(struct inode *, struct dentry *, int);
    struct dentry *(*lookup)(struct inode *, struct dentry *);
};

struct inode {
    ino_t i_ino;
    mode_t i_mode;
    uid_t i_uid;
    gid_t i_gid;
    size_t i_size;
    time_t i_atime;
    time_t i_mtime;
    time_t i_ctime;
    uint32_t i_flags;
    struct inode_operations *i_ops;
    struct superblock *i_sb;
};

struct dentry {
    struct inode *d_parent;
    struct superblock *d_sb;
    ino_t d_ino;
    char d_name[DIRENT_NAME_MAX];
};

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

struct superblock_operations {
    struct inode *(*alloc_inode)(struct superblock *);
    int (*destroy_inode)(struct inode *);
    void (*read_inode)(struct inode *);
    void (*write_inode)(struct inode *, int);
};

struct superblock {
    struct superblock_operations *s_ops;
    struct dentry *s_root;
};
