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
#include <fs/path.h>
#include <string.h>

struct file* file_allocate(void)
{
    struct file* file = kmalloc(sizeof(struct file));
    if (file)
        memset(file, 0, sizeof(struct file));
    return file;
}

void file_free(struct file* file)
{
    if (file)
        free((void*)file);
}

ssize_t file_pread(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file || !buffer)
        return -EIO;
    if (file->f_ops->read)
        return file->f_ops->read(file, buffer, size, file->f_off);
    else
        return -EIO;
}

ssize_t file_read(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file || !buffer)
        return -EIO;
    ssize_t ret = file_pread(file, buffer, size);
    if (ret > 0)
        file->f_off += ret;
    return ret;
}

ssize_t file_pwrite(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file || !buffer)
        return -EIO;
    if (file->f_ops->write)
        return file->f_ops->write(file, buffer, size, file->f_off);
    else
        return -EIO;
}

ssize_t file_write(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file || !buffer)
        return -EIO;
    ssize_t ret = file_pwrite(file, buffer, size);
    if (ret > 0)
        file->f_off += ret;
    return ret;
}

struct file* file_open(const char* name, uint32_t flags, mode_t mode)
{
    struct inode* inode = NULL;
    struct dentry* dentry = NULL;
    inode = path_resolve(name, flags, mode, &dentry);
    if (!inode || !dentry)
        return NULL;
    struct file* file = file_allocate();
    if (!file)
        return NULL;
    file->f_inode = inode;
    file->f_dentry = dentry;
    file->f_ops = inode->i_fops;
    return file;
}

void file_init(void)
{
}
