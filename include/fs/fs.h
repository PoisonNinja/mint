/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <block/block.h>
#include <fs/device.h>
#include <lib/list.h>
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
struct fs_device;
struct mountpoint;

struct mountpoint {
    struct inode *mp_inode;
    struct superblock *mp_sb;
    struct list_element list;
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
    struct list_element i_mp;
    struct fs_device *i_fs_dev;
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
    struct list_element list;
};

extern void filesystem_init(void);
extern void rootfs_init(void);

extern int filesystem_register(struct filesystem *fs);
extern int filesystem_unregister(struct filesystem *fs);
extern struct filesystem *filesystem_get(const char *name);
