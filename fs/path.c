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
    struct inode* dir = __path_resolve(start, dirpath, 0, NULL);
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
        return __path_resolve_create(start, path, flags, mode, dentry);
    } else {
        return __path_resolve(start, path, flags, dentry);
    }
}
