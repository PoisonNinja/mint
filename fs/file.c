#include <fs/file.h>
#include <fs/open.h>
#include <mm/slab.h>

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

void file_init(void)
{
    file_slab_cache = slab_create("file_cache", sizeof(struct file), 0);
}
