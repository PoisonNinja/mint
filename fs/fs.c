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
#include <fs/mount.h>
#include <fs/path.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/process.h>

static struct list_element fs_list = LIST_COMPILE_INIT(fs_list);

int filesystem_register(struct filesystem* fs)
{
    printk(INFO, "fs: Adding filesystem %s with data structure at %p\n",
           fs->name, fs);
    list_add(&fs_list, &fs->list);
    return 0;
}

int filesystem_unregister(struct filesystem* fs)
{
    list_delete(&fs->list);
    return 0;
}

struct filesystem* filesystem_get(const char* name)
{
    struct filesystem* fs = NULL;
    list_for_each(&fs_list, list, fs)
    {
        if (!strcmp(fs->name, name))
            return fs;
    }
    return NULL;
}

extern void inode_init(void);

void filesystem_init(void)
{
    inode_init();
}

extern struct inode* fs_root;

void rootfs_init(void)
{
    struct filesystem* fs = filesystem_get("initfs");
    if (!fs) {
        printk(WARNING, "WTF is this, no initfs?\n");
        return;
    }
    struct superblock* sb = kmalloc(sizeof(struct superblock));
    int r = 0;
    if ((r = fs->mount(sb)) < 0) {
        kfree(sb);
        return;
    }
    current_process->root = current_process->cwd = sb->s_root;
    if (!path_resolve("/dev", O_CREAT, S_IFDIR, NULL)) {
        printk(ERROR, "Failed to create /dev\n");
    }
    return;
}
