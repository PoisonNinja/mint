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
#include <fs/fs.h>
#include <fs/mount.h>
#include <fs/path.h>
#include <kernel.h>
#include <lib/list.h>
#include <mm/heap.h>

int mount_fs(const char* special, const char* mountpoint, const char* name,
             uint32_t flags)
{
    struct filesystem* fs = filesystem_get(name);
    if (!fs) {
        printk(WARNING, "Invalid filesystem requested: %s\n", name);
        return -ENODEV;
    }
    // Sadly, no slab_cache for you
    struct superblock* sb = kmalloc(sizeof(struct superblock));
    int r = 0;
    if ((r = fs->mount(sb)) < 0) {
        kfree(sb);
        return r;
    }
    struct mountpoint* mp = kmalloc(sizeof(struct mountpoint));
    mp->mp_sb = sb;
    mp->mp_inode = sb->s_root;
    struct inode* mountpoint_inode = path_resolve(mountpoint, 0, 0, NULL);
    if (!mountpoint_inode) {
        kfree(mp);
        kfree(sb);
        return -ENOENT;
    }
    list_add(&mountpoint_inode->i_mp, &mp->list);
    return 0;
}
