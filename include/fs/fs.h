#pragma once

#include <types.h>

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_APPEND 0x0008
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
#define O_EXCL 0x0800
#define O_NOFOLLOW 0x1000
#define O_PATH 0x2000

#define DENTRY_NAME_MAX 256

struct inode;
struct dentry;
struct file;
struct superblock;
struct mountpoint;

struct mountpoint {
    struct inode *mp_inode;
    struct superblock *mp_sb;
    struct mountpoint *next, *prev;
};

struct inode_operations {
    struct inode *(*create)(struct inode *, struct dentry *, int, mode_t);
    int (*lookup)(struct inode *, struct dentry *);
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
    struct file_operations *i_fops;
    struct superblock *i_sb;
    struct mountpoint *i_mp;
};

struct dentry {
    struct inode *d_parent;
    struct superblock *d_sb;
    ino_t d_ino;
    char d_name[DENTRY_NAME_MAX];
};

struct superblock_operations {
    struct inode *(*alloc_inode)(struct superblock *);
    int (*read_inode)(struct inode *);
    int (*write_inode)(struct inode *);
};

struct superblock {
    struct superblock_operations *s_ops;
    struct inode *s_root;
    void *s_data;
};

struct filesystem {
    const char *name;
    int (*mount)(struct superblock *);
    struct filesystem *next, *prev;
};

extern int filesystem_register(struct filesystem *fs);
extern int filesystem_unregister(struct filesystem *fs);
extern struct filesystem *filesystem_get(const char *name);
