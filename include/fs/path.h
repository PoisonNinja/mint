#pragma once

#include <fs/fs.h>

/*
 * path_resolve: Resolve a path into a inode and optionally a dentry
 */
extern struct inode *path_resolve(const char *path, uint32_t flags, mode_t mode,
                                  struct dentry **dentry);
