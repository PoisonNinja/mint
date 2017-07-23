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

char* dirname(char* path)
{
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
                                           const char* path, mode_t mode)
{
    if (!start || !path)
        return NULL;
    char* dirpath = dirname(path);
    char* filename = basename(path);
    struct inode* dir = path_resolve(dirpath, 0, mode);
    if (!dir)
        return NULL;
    if (!S_ISDIR(dir->i_mode))
        return NULL;
    struct dentry* dentry = NULL;
    if ((dentry = dentry_lookup(dir, filename))) {
        struct inode* ret = inode_resolve_dentry(dentry);
        return ret;
    }
    dentry = kmalloc(sizeof(struct dentry));
    strncpy(dentry->d_name, filename, DENTRY_NAME_MAX);
    return dir->i_ops->create(dir, dentry, mode);
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

struct inode* path_resolve(const char* path, uint32_t flags, mode_t mode)
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
    if (flags & O_CREAT) {
        return __path_resolve_create(start, path, mode);
    } else {
        struct dentry* dentry = __path_resolve_dentry(start, path);
        if (!dentry)
            return NULL;
        struct inode* inode = inode_resolve_dentry(dentry);
        return inode;
    }
}
