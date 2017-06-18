/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/boot/multiboot.h>
#include <arch/cpu/cpu.h>
#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <mm/heap.h>
#include <string.h>
#include <types.h>

extern int x86_64_init_console(void);
extern void kmain(struct mint_bootinfo *);

extern void gdt_init(void);
extern void idt_init(void);

static struct mint_bootinfo bootinfo;

extern uint64_t __bss_start;
extern uint64_t __bss_end;

extern uint64_t __kernel_end;

void zero_bss(void)
{
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}

void x86_64_init(uint32_t magic, struct multiboot_info *mboot)
{
    interrupt_disable();
    zero_bss();
    x86_64_init_console();
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Bad multiboot magic! Expected 0x%X, but got 0x%X\n",
              MULTIBOOT_BOOTLOADER_MAGIC, magic);
    printk(INFO, "x86_64-init: Multiboot information structure at 0x%llX\n",
           mboot);
    cpu_initialize_information();
    cpu_print_information(cpu_get_information(0));
    gdt_init();
    idt_init();
    uint32_t mmap = mboot->mmap_addr;
    while (mmap < mboot->mmap_addr + mboot->mmap_length) {
        multiboot_memory_map_t *tmp = (multiboot_memory_map_t *)(uint64_t)mmap;
        if (tmp->type == 1) {
            /*
             * We manually calculate total_mem because upper_mem from
             * multiboot only goes up to the first memory hole at around
             * 3GB. This could be a QEMU bug though...
             */
            bootinfo.total_mem += tmp->len;
        }
        bootinfo.num_memregions++;
        mmap += (tmp->size + sizeof(tmp->size));
    }
    /*
     * Tell early_malloc where it can allocate memory from and the extent that
     * it can allocate to
     */
    early_malloc_set_properties((uint64_t)&__kernel_end, 0x10000);
    kmain(&bootinfo);
}
