#include <errno.h>
#include <fs/file.h>
#include <fs/path.h>
#include <mm/slab.h>
#include <string.h>

static struct slab_cache* file_slab_cache = NULL;

struct file* file_allocate(void)
{
    struct file* file = slab_allocate(file_slab_cache);
    if (file)
        memset(file, 0, sizeof(struct file));
    return file;
}

void file_free(struct file* file)
{
    if (file)
        slab_free((void*)file);
}

ssize_t file_pread(struct file* file, uint8_t* buffer, size_t size)
{
    if (file->f_ops->read)
        return file->f_ops->read(file, buffer, size);
    else
        return -EIO;
}

ssize_t file_read(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file)
        return -EIO;
    ssize_t ret = file_pread(file, buffer, size);
    if (ret > 0)
        file->f_off += ret;
    return ret;
}

ssize_t file_pwrite(struct file* file, uint8_t* buffer, size_t size)
{
    if (file->f_ops->write)
        return file->f_ops->write(file, buffer, size);
    else
        return -EIO;
}

ssize_t file_write(struct file* file, uint8_t* buffer, size_t size)
{
    if (!file)
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
    file_slab_cache = slab_create("file_cache", sizeof(struct file), 0);
}
