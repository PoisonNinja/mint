#pragma once

#include <lib/list.h>
#include <types.h>

extern void ksymbol_register(addr_t address, char* name, size_t name_size);

struct ksymbol_resolve {
    addr_t address;
    char* name;
    size_t offset;
};
extern struct ksymbol_resolve* ksymbol_resolve(addr_t address);
