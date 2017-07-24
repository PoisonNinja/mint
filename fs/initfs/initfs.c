#include <errno.h>
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
    struct initfs_dirent *next, *prev;
};

struct initfs_inode {
    mode_t i_mode;
    ino_t i_ino;
    uid_t i_uid;
    gid_t i_gid;
    size_t i_size;
    nlink_t i_nlink;
    void* i_data;
    struct initfs_dirent* i_entries;
};

struct initfs_data {
    struct initfs_inode* inodes[MAX_INODES];
    ino_t next;
};

struct inode* initfs_create(struct inode* parent, struct dentry* dentry,
                            int flags)
{
    struct initfs_data* data = (struct initfs_data*)parent->i_sb->s_data;
    struct initfs_inode* rnode = kzalloc(sizeof(struct initfs_inode));
    struct inode* inode = kmalloc(sizeof(struct inode));
    struct initfs_dirent* dir = kmalloc(sizeof(struct initfs_dirent));
    rnode->i_ino = data->next++;
    inode->i_ino = rnode->i_ino;
    dentry->d_ino = inode->i_ino;
    dir->d_ino = inode->i_ino;
    strncpy(dir->d_name, dentry->d_name, DENTRY_NAME_MAX);
    data->inodes[rnode->i_ino] = rnode;
    LIST_PREPEND(rnode->i_entries, dir);
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
    LIST_FOR_EACH(rnode->i_entries, rdirent)
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
    return 0;
}

struct superblock_operations initfs_superblock_operations = {
    .read_inode = initfs_read_inode, .write_inode = initfs_write_inode,
};

static int initfs_mount(struct superblock* sb)
{
    struct initfs_data* data = kmalloc(sizeof(struct initfs_data));
    sb->s_data = data;
    sb->s_ops = &initfs_superblock_operations;
    struct inode* root_inode = inode_allocate(sb);
    root_inode->i_ino = 0;
    root_inode->i_ops = &initfs_inode_operations;
    root_inode->i_sb = sb;
    root_inode->i_mode = S_IFDIR;
    sb->s_root = root_inode;
    data->inodes[0] = kmalloc(sizeof(struct initfs_inode));
    struct initfs_inode* root = data->inodes[0];
    root->i_ino = 0;
    root->i_mode = S_IFDIR;
    return 0;
}

static struct filesystem initfs_filesystem = {
    .name = "initfs", .mount = initfs_mount,
};

static int init_initfs(void)
{
    filesystem_register(&initfs_filesystem);
    return 0;
}
FS_INITCALL(init_initfs);
