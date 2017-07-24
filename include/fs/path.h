#pragma once

#include <fs/fs.h>

/*
 * path_resolve(): Resolves a path into a inode and optionally a dentry
 *
 * path: Path that you want to resolve. It can be relative, and will behave
 * similar to what you would expect in userspace.
 * flags: O_* flags
 * mode: Used for when O_CREAT is enabled, ignored otherwise
 * dentry: Pointer to a dentry struct pointer, will be filled out with the
 * dentry of the inode returned. The pointer can be NULL if the dentry is not
 * required.
 *
 * Returns the inode if found or created (if O_CREAT is used)
 */
extern struct inode *path_resolve(const char *path, int flags, mode_t mode,
                                  struct dentry **dentry);
