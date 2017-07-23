#pragma once

#include <fs/fs.h>

extern struct dentry *path_resolve_dentry(const char *path, uint32_t flags);
extern struct inode *path_resolve(const char *path, uint32_t flags,
                                  mode_t mode);
