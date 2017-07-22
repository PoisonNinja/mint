#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/path.h>
#include <kernel.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/process.h>

struct inode* fs_root = NULL;

static struct dentry* __path_resolve(struct inode* start, const char* path,
                                     uint32_t flags)
{
    struct inode* inode = start;
    struct dentry* dentry = NULL;
    char* str = (char*)path;
    char* name;
    while (inode && (name = strtok_r(str, "/", &str))) {
        dentry = dentry_lookup(inode, name);
        if (!dentry)
            return NULL;
        inode = inode_resolve_dentry(dentry);
    }
    return dentry;
}

struct dentry* path_resolve(const char* path, uint32_t flags)
{
    if (!path)
        return NULL;
    struct inode* inode = NULL;
    // if (*path == '/') {
    //     inode = current_process->root;
    //     path++;
    // } else {
    //     inode = current_process->cwd;
    // }
    if (*path == '/')
        path++;
    inode = fs_root;
    return __path_resolve(inode, path, flags);
}

struct inode* path_resolve_inode(const char* path, uint32_t flags)
{
    struct dentry* dentry = path_resolve(path, flags);
    if (!dentry)
        return NULL;
    struct inode* inode = inode_resolve_dentry(dentry);
    return inode;
}
