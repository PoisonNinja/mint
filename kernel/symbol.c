#include <kernel/symbol.h>
#include <mm/heap.h>
#include <string.h>

struct ksymbol {
    addr_t address;
    struct list_element list;
    char name[];
};

struct list_element ksymbol_list = LIST_COMPILE_INIT(ksymbol_list);

void ksymbol_register(addr_t address, char* name, size_t name_size)
{
    struct ksymbol* ksymbol = kmalloc(sizeof(struct ksymbol) + name_size + 2);
    ksymbol->address = address;
    strncpy(ksymbol->name, name, name_size);
    ksymbol->name[name_size] = '\0';
    list_add(&ksymbol_list, &ksymbol->list);
}

static char unknown[] = "??????????????????";
static struct ksymbol_resolve resolve;

struct ksymbol_resolve* ksymbol_resolve(addr_t address)
{
    size_t difference = ~0;
    resolve.address = 0;
    resolve.name = &unknown;
    resolve.offset = 0;
    struct ksymbol* ksymbol = NULL;
    list_for_each(&ksymbol_list, list, ksymbol)
    {
        if (ksymbol->address <= address) {
            size_t temp = address - ksymbol->address;
            if (temp < difference) {
                resolve.address = ksymbol->address;
                resolve.name = ksymbol->name;
                resolve.offset = temp;
                difference = temp;
            }
        }
    }
    return &resolve;
}
