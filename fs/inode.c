#include <fs/fs.h>
#include <fs/inode.h>
#include <mm/slab.h>
#include <string.h>

static struct slab_cache* inode_slab_cache = NULL;

struct inode* inode_allocate(void)
{
    struct inode* inode = slab_allocate(inode_slab_cache);
    if (inode)
        memset(inode, 0, sizeof(struct inode));
    return inode;
}

struct inode* inode_resolve_dentry(struct dentry* dentry)
{
    if (!dentry)
        return NULL;
    struct inode* inode = inode_allocate();
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
    inode_slab_cache = slab_create("inode_cache", sizeof(struct inode), 0);
}
