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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <cpu/power.h>
#include <drivers/pci/pci.h>
#include <fs/fs.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/stacktrace.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <mm/mm.h>
#include <tm/sched.h>

extern void setup_arch(void);

void do_initcalls(void)
{
}

void kmain(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%s\n", OS_STRING);
    mm_init(bootinfo);
    setup_arch();
    time_init();
    sched_init();
    filesystem_init();
    pci_init();
    interrupt_enable();
    do_initcall(EARLY_INIT);
    do_initcall(CORE_INIT);
    do_initcall(ARCH_INIT);
    do_initcall(SUBSYS_INIT);
    do_initcall(FS_INIT);
    rootfs_init();
    do_initcall(DEVICE_INIT);
    do_initcall(LATE_INIT);
    do_initcall(TEST_INIT);
    for (;;)
        cpu_halt();
}
