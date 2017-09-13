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

#include <errno.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/stat.h>
#include <kernel/init.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <string.h>

#define MAX_INODES 1000

struct initfs_dirent {
    char d_name[DENTRY_NAME_MAX];
    ino_t d_ino;
    struct list_element list;
};

struct initfs_inode {
    mode_t i_mode;
    ino_t i_ino;
    uid_t i_uid;
    gid_t i_gid;
    size_t i_size;
    nlink_t i_nlink;
    void* i_data;
    struct list_element i_entries;
};

struct initfs_data {
    struct initfs_inode* inodes[MAX_INODES];
    ino_t next;
};

static struct inode* initfs_allocate(struct superblock* sb);

static ssize_t initfs_read(struct file* file, uint8_t* buffer, size_t size,
                           off_t offset)
{
    struct initfs_data* data = (struct initfs_data*)file->f_inode->i_sb->s_data;
    if (!data)
        return -EIO;
    struct initfs_inode* rnode = data->inodes[file->f_inode->i_ino];
    if (!rnode)
        return -EIO;
    ssize_t total = size + offset;
    if ((size_t)total > rnode->i_size)
        total = rnode->i_size - offset;
    memcpy(buffer, rnode->i_data + offset, total);
    return total;
}

static ssize_t initfs_write(struct file* file, uint8_t* buffer, size_t size,
                            off_t offset)
{
    struct initfs_data* data = (struct initfs_data*)file->f_inode->i_sb->s_data;
    if (!data)
        return -EIO;
    struct initfs_inode* rnode = data->inodes[file->f_inode->i_ino];
    if (!rnode)
        return -EIO;
    ssize_t total = size + offset;
    void* new = kmalloc(total);
    if (rnode->i_data) {
        memcpy(new, rnode->i_data, rnode->i_size);
        kfree(rnode->i_data);
    }
    rnode->i_data = new;
    void* tmp = rnode->i_data + offset;
    memcpy(tmp, buffer, size);
    rnode->i_size += size;
    file->f_inode->i_size += size;
    return size;
}

static struct file_operations initfs_file_operations = {
    .read = &initfs_read, .write = &initfs_write,
};

struct inode* initfs_create(struct inode* parent, struct dentry* dentry,
                            int flags, mode_t mode)
{
    struct initfs_data* data = (struct initfs_data*)parent->i_sb->s_data;
    struct initfs_inode* rnode = kzalloc(sizeof(struct initfs_inode));
    struct inode* inode = initfs_allocate(parent->i_sb);
    struct initfs_dirent* dir = kmalloc(sizeof(struct initfs_dirent));
    rnode->i_ino = data->next++;
    list_runtime_init(&rnode->i_entries);
    inode->i_ino = rnode->i_ino;
    inode->i_mode = mode;
    dentry->d_ino = inode->i_ino;
    dir->d_ino = inode->i_ino;
    strncpy(dir->d_name, dentry->d_name, DENTRY_NAME_MAX);
    data->inodes[rnode->i_ino] = rnode;
    rnode->i_mode = mode;
    list_add(&rnode->i_entries, &dir->list);
    return inode;
}

static int initfs_lookup(struct inode* root, struct dentry* entry)
{
    struct initfs_data* data = (struct initfs_data*)root->i_sb->s_data;
    if (!data)
        return -EIO;
    struct initfs_inode* rnode = data->inodes[root->i_ino];
    if (!rnode)
        return -EIO;
    struct initfs_dirent* rdirent = NULL;
    list_for_each(&rnode->i_entries, list, rdirent)
    {
        if (!strcmp(rdirent->d_name, entry->d_name)) {
            entry->d_ino = rdirent->d_ino;
            entry->d_sb = root->i_sb;
            return 0;
        }
    }
    return -ENOENT;
}

static struct inode_operations initfs_inode_operations = {
    .create = &initfs_create, .lookup = &initfs_lookup,
};

static int initfs_write_inode(struct inode* node)
{
    struct initfs_data* data = (struct initfs_data*)node->i_sb->s_data;
    if (!data)
        return -EIO;
    struct initfs_inode* inode = data->inodes[node->i_ino];
    if (!inode)
        return -EIO;
    inode->i_mode = node->i_mode;
    inode->i_ino = node->i_ino;
    inode->i_uid = node->i_uid;
    inode->i_gid = node->i_gid;
    inode->i_size = node->i_size;
    return 0;
}

static int initfs_read_inode(struct inode* node)
{
    struct initfs_data* data = (struct initfs_data*)node->i_sb->s_data;
    if (!data)
        return -EIO;
    struct initfs_inode* inode = data->inodes[node->i_ino];
    if (!inode)
        return -EIO;
    node->i_mode = inode->i_mode;
    node->i_ino = inode->i_ino;
    node->i_uid = inode->i_uid;
    node->i_gid = inode->i_gid;
    node->i_size = inode->i_size;
    node->i_ops = &initfs_inode_operations;
    node->i_fops = &initfs_file_operations;
    return 0;
}

struct superblock_operations initfs_superblock_operations = {
    .read_inode = initfs_read_inode, .write_inode = initfs_write_inode,
};

static int initfs_mount(struct superblock* sb)
{
    struct initfs_data* data = kmalloc(sizeof(struct initfs_data));
    data->next = 0;
    sb->s_data = data;
    sb->s_ops = &initfs_superblock_operations;
    struct inode* root_inode = initfs_allocate(sb);
    root_inode->i_ino = 0;
    root_inode->i_mode = S_IFDIR;
    sb->s_root = root_inode;
    data->inodes[0] = kmalloc(sizeof(struct initfs_inode));
    struct initfs_inode* root = data->inodes[0];
    root->i_ino = 0;
    root->i_mode = S_IFDIR;
    list_runtime_init(&root->i_entries);
    return 0;
}

static struct filesystem initfs_filesystem = {
    .name = "initfs", .mount = initfs_mount,
};

static struct inode* initfs_allocate(struct superblock* sb)
{
    struct inode* inode = inode_allocate(sb);
    inode->i_ops = &initfs_inode_operations;
    inode->i_fops = &initfs_file_operations;
    inode->i_sb = sb;
    return inode;
}

static int init_initfs(void)
{
    filesystem_register(&initfs_filesystem);
    return 0;
}
FS_INITCALL(init_initfs);
