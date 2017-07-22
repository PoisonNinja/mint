#pragma once

#include <fs/fs.h>

extern int mount_fs(const char* special, const char* mountpoint,
                    const char* name, uint32_t flags);
