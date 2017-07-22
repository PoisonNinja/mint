#pragma once

#include <fs/fs.h>

extern struct inode* inode_allocate(struct superblock* sb);
extern struct inode* inode_resolve_dentry(struct dentry* dentry);
