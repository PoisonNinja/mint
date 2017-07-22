#pragma once

#include <fs/fs.h>

extern struct dentry* path_resolve(const char* path, uint32_t flags);
extern struct inode* path_resolve_inode(const char* path, uint32_t flags);
