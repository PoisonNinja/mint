#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/path.h>
#include <kernel.h>
#include <mm/heap.h>
#include <string.h>
#include <tm/process.h>

struct inode* fs_root = NULL;

static char* dirname(const char* path)
{
    char* newpath;
    char* slash;
    int length; /* Length of result, not including NUL.  */

    slash = strrchr(path, '/');
    if (slash == 0) {
        /* File is in the current directory.  */
        path = "";
        length = 0;
    } else {
        /* Remove any trailing slashes from the result.
        while (slash > path && *slash == '/')
        --slash; */

        length = slash - path + 1;
    }
    newpath = (char*)kmalloc(length + 1);
    if (newpath == 0)
        return 0;
    strncpy(newpath, path, length);
    newpath[length] = 0;
    return newpath;
}

static char* basename(char const* name)
{
    char const* base = name;
    int all_slashes = 1;
    char const* p;

    for (p = name; *p; p++) {
        if (*p == '/')
            base = p + 1;
        else
            all_slashes = 0;
    }

    /* If NAME is all slashes, arrange to return `/'.  */
    if (*base == '\0' && *p == '/' && all_slashes)
        --base;

    return (char*)base;
}

static inline struct inode* path_resolve_mountpoint(struct inode* original)
{
    if (original->i_mp) {
        return original->i_mp->mp_inode;
    } else {
        return original;
    }
}

static struct dentry* __path_resolve_dentry(struct inode* start,
                                            const char* path)
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

static struct inode* __path_resolve_create(struct inode* start,
                                           const char* path)
{
    if (!start || !path)
        return NULL;
    char* dirpath = dirname(path);
    char* filename = basename(path);
    struct inode* dir = path_resolve(dirpath, 0);
    if (!dir)
        return NULL;
    struct dentry* dentry = NULL;
    if ((dentry = dentry_lookup(dir, filename))) {
        struct inode* ret = inode_resolve_dentry(dentry);
        return ret;
    }
    dentry = kmalloc(sizeof(struct dentry));
    strncpy(dentry->d_name, filename, DENTRY_NAME_MAX);
    return dir->i_ops->create(dir, dentry, 0);
}

struct dentry* path_resolve_dentry(const char* path, uint32_t flags)
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
    return __path_resolve_dentry(inode, path);
}

struct inode* path_resolve(const char* path, uint32_t flags)
{
    if (!strcmp(path, "/"))
        return fs_root;
    struct inode* start = NULL;
    // if (*path == '/') {
    //     inode = current_process->root;
    //     path++;
    // } else {
    //     inode = current_process->cwd;
    // }
    if (*path == '/')
        path++;
    start = fs_root;
    struct dentry* dentry = __path_resolve_dentry(start, path);
    if (!dentry)
        return NULL;
    struct inode* inode = inode_resolve_dentry(dentry);
    return inode;
}
