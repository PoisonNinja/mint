/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja)
 *
 * This file is part of Mint. Mint is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/stacktrace.h>
#include <kernel/time/time.h>
#include <kernel/version.h>

static char mint_banner[] =
    "Mint version " UTS_RELEASE " (" MINT_COMPILE_BY "@" MINT_COMPILE_HOST
    ") (" MINT_COMPILER ") " UTS_VERSION;

extern void setup_arch(void);

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", mint_banner);
    printk(INFO, "%llu KiB of memory available\n", bootinfo->total_mem);
    setup_arch();
    time_init();
    interrupt_enable();
    do_initcall(EARLY_INIT);
    do_initcall(CORE_INIT);
    do_initcall(ARCH_INIT);
    do_initcall(SUBSYS_INIT);
    do_initcall(FS_INIT);
    do_initcall(DEVICE_INIT);
    do_initcall(LATE_INIT);
    stacktrace();
    for (;;)
        ;
}
