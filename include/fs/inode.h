#pragma once

#include <fs/fs.h>

extern struct inode* inode_allocate(void);
extern struct inode* inode_resolve_dentry(struct dentry* dentry);
