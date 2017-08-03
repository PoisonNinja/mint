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
