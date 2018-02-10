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

#include <fs/fs.h>
#include <fs/inode.h>
#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <string.h>

static struct hashmap* inode_cache = NULL;

struct inode* inode_check_cache(struct superblock* sb, ino_t ino)
{
    uint64_t key[2] = {(uint64_t)sb, ino};
    struct inode* ret = hashmap_get(inode_cache, key, 16);
    return ret;
}

void inode_insert_cache(struct superblock* sb, struct inode* inode)
{
    uint64_t key[2] = {(uint64_t)sb, inode->i_ino};
    hashmap_set(inode_cache, key, 16, inode);
}

struct inode* inode_allocate(struct superblock* sb)
{
    if (sb->s_ops->alloc_inode) {
        return sb->s_ops->alloc_inode(sb);
    } else {
        struct inode* inode = kmalloc(sizeof(struct inode));
        if (inode) {
            memset(inode, 0, sizeof(struct inode));
            list_runtime_init(&inode->i_mp);
        }
        return inode;
    }
}

struct inode* inode_resolve_dentry(struct dentry* dentry)
{
    if (!dentry)
        return NULL;
    struct inode* inode = NULL;
    if ((inode = inode_check_cache(dentry->d_sb, dentry->d_ino)))
        return inode;
    inode = inode_allocate(dentry->d_sb);
    if (!inode)
        return NULL;
    inode->i_ino = dentry->d_ino;
    inode->i_sb = dentry->d_sb;
    if (inode->i_sb->s_ops->read_inode)
        inode->i_sb->s_ops->read_inode(inode);
    return inode;
}

void inode_init(void)
{
    inode_cache = hashmap_create(0x4000);
}
