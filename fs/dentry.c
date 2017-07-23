#include <fs/fs.h>
#include <mm/slab.h>
#include <string.h>

static struct slab_cache* dentry_slab_cache = NULL;

struct dentry* dentry_allocate(void)
{
    struct dentry* dentry = slab_allocate(dentry_slab_cache);
    if (dentry)
        memset(dentry, 0, sizeof(struct dentry));
    return dentry;
}

void dentry_free(struct dentry* dentry)
{
    if (!dentry)
        return;
    slab_free(dentry);
}

struct dentry* dentry_lookup(struct inode* inode, char* name)
{
    if (!inode || !name)
        return NULL;
    struct dentry* dentry = dentry_allocate();
    if (!dentry)
        return NULL;
    strncpy(dentry->d_name, name, DENTRY_NAME_MAX);
    if (inode->i_ops->lookup(inode, dentry) >= 0) {
        return dentry;
    } else {
        dentry_free(dentry);
        return NULL;
    }
}

void dentry_init(void)
{
    dentry_slab_cache = slab_create("dentry_cache", sizeof(struct dentry), 0);
}
