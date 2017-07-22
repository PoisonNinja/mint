#pragma once

#include <fs/fs.h>

extern struct dentry* dentry_lookup(struct inode* inode, char* name);
