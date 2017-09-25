#include <fs/device.h>
#include <fs/path.h>

struct inode* mknod(const char* path, mode_t mode, dev_t dev)
{
    struct inode* ret = path_resolve(path, O_CREAT, mode, NULL);
    if (!ret) {
        return NULL;
    }
    ret->i_mode = mode;
    ret->i_uid = 0;
    return ret;
}
