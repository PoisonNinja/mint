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

#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/path.h>
#include <fs/stat.h>
#include <kernel.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/process.h>

struct inode* fs_root = NULL;

char* dirname(const char* original)
{
    char* path = strdup(original);
    char* p;
    if (path == NULL || *path == '\0')
        return "/";
    p = path + strlen(path) - 1;
    while (*p == '/') {
        if (p == path)
            return path;
        *p-- = '\0';
    }
    while (p >= path && *p != '/')
        p--;
    return p < path ? "/" : p == path ? "/" : (*p = '\0', path);
}

static char* basename(const char* name)
{
    const char* base = name;

    while (*name) {
        if (*name++ == '/') {
            base = name;
        }
    }
    return (char*)base;
}

static inline struct inode* path_resolve_mountpoint(struct inode* original)
{
    if (!list_empty(&original->i_mp)) {
        struct mountpoint* mp =
            list_first_entry(&original->i_mp, struct mountpoint, list);
        return mp->mp_inode;
    } else {
        return original;
    }
}

static struct dentry* __path_resolve_dentry(struct inode* start,
                                            const char* path, int flags)
{
    if (!start || !path)
        return NULL;
    struct inode* inode = start;
    struct dentry* dentry = NULL;
    char* str = (char*)path;
    char* name;
    while (inode && (name = strtok_r(str, "/", &str))) {
        dentry = dentry_lookup(inode, name);
        if (!dentry)
            return NULL;
        inode = inode_resolve_dentry(dentry);
        inode = path_resolve_mountpoint(inode);
    }
    return dentry;
}

static struct inode* __path_resolve(struct inode* start, const char* path,
                                    int flags, struct dentry** dentry)
{
    struct dentry* fdentry = __path_resolve_dentry(start, path, flags);
    if (!fdentry)
        return NULL;
    struct inode* inode = inode_resolve_dentry(fdentry);
    if (dentry)
        *dentry = fdentry;
    return inode;
}

static struct inode* __path_resolve_create(struct inode* start,
                                           const char* path, int flags,
                                           mode_t mode, struct dentry** dentry)
{
    if (!start || !path)
        return NULL;
    char* dirpath = dirname(path);
    char* filename = basename(path);
    /*
     * Get the directory inode using path_resolve. Notice that we're not
     * using __path_resolve because path_resolve does some special
     * processing for "/", which __path_resolve can't handle.
     *
     * So, we call path_resolve, passing in the directory path and a copy
     * of the original flags, excluding O_CREATE because I don't want
     * to accidentally create the directory too.
     */
    struct inode* dir = path_resolve(dirpath, 0, (flags & ~O_CREAT), NULL);
    if (!dir)
        return NULL;
    if (!S_ISDIR(dir->i_mode))
        return NULL;
    struct dentry* new = NULL;
    if ((new = dentry_lookup(dir, filename))) {
        struct inode* ret = inode_resolve_dentry(new);
        return ret;
    }
    new = kmalloc(sizeof(struct dentry));
    strncpy(new->d_name, filename, DENTRY_NAME_MAX);
    if (dentry)
        *dentry = new;
    return dir->i_ops->create(dir, new, flags, mode);
}

struct inode* path_resolve(const char* path, int flags, mode_t mode,
                           struct dentry** dentry)
{
    struct inode* start = NULL;
    if (*path == '/') {
        /*
         * A quick check to see if the entire path only consists of /. Instead
         * of using strlen which can get really bad performance (O(n)) for
         * longer strings, we simply check if the next character is a NULL
         * terminator.
         */
        if (*(path + 1) == '\0')
            return current_process->root;
        start = current_process->root;
        path++;
    } else {
        start = current_process->cwd;
    }
    if (flags & O_CREAT) {
        return __path_resolve_create(start, path, flags, mode, dentry);
    } else {
        return __path_resolve(start, path, flags, dentry);
    }
}
