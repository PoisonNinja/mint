#include <arch/cpu/tables.h>
#include <kernel.h>
#include <string.h>

#define NUM_ENTRIES 3

static struct gdt_entry gdt_entries[NUM_ENTRIES];
static struct gdt_descriptor gdt_ptr;

extern void gdt_flush(uint64_t);

static void gdt_set_entry(struct gdt_entry* entry, uint32_t base,
                          uint32_t limit, uint8_t access, uint8_t flags)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->flags = flags & 0xF;
    entry->base_high = (base >> 24) & 0xFF;
}

void gdt_init(void)
{
    memset(&gdt_entries, 0, sizeof(struct gdt_entry) * NUM_ENTRIES);
    memset(&gdt_ptr, 0, sizeof(struct gdt_descriptor));
    gdt_set_entry(&gdt_entries[0], 0, 0, 0, 0);
    gdt_set_entry(&gdt_entries[1], 0, 0xFFFFF, 0x9A, 0x0A);
    gdt_set_entry(&gdt_entries[2], 0, 0xFFFFF, 0x92, 0x0A);
    gdt_ptr.limit = sizeof(struct gdt_entry) * NUM_ENTRIES - 1;
    gdt_ptr.offset = (uint64_t)gdt_entries;
    gdt_flush((uint64_t)&gdt_ptr);
}
