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

#include <arch/cpu/cpu.h>
#include <kernel.h>
#include <string.h>

struct cpuinfo bsp_info;

struct __attribute__((packed, aligned(8))) {
    uint32_t eax, ebx, ecx, edx;
} regs;

void cpu_print_information(struct cpuinfo* cpu)
{
    printk(INFO, "CPU information: \n");
    printk(INFO, "  Signature: %s\n", cpu->signature);
    printk(INFO, "  Name: %s\n", cpu->processor_name);
    printk(INFO, "  Stepping: 0x%X\n", cpu->stepping);
    printk(INFO, "  Type: 0x%X\n", cpu->type);
    printk(INFO, "  Family: 0x%X\n", cpu->family);
    printk(INFO, "  Model: 0x%X\n", cpu->model);
    printk(INFO, "  Features: 0x%08X 0x%08X\n", cpu->features[0],
           cpu->features[1]);
    printk(INFO, "  Extended Features: 0x%08X 0x%08X\n", cpu->features[2],
           cpu->features[3]);
    printk(INFO, "  Invariant TSC: %s\n",
           (cpu->invariant_tsc) ? "true" : "false");
}

void cpu_initialize_information(void)
{
    regs.eax = 0x0;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    *(uint32_t*)&bsp_info.signature[0] = regs.ebx;
    *(uint32_t*)&bsp_info.signature[4] = regs.edx;
    *(uint32_t*)&bsp_info.signature[8] = regs.ecx;
    // Manually add null terminator to prevent buffer overruns
    bsp_info.signature[12] = '\0';
    regs.eax = 0x1;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    bsp_info.stepping = regs.eax & 0xF;
    bsp_info.type = regs.eax & 0x3000;
    bsp_info.family = (regs.eax & 0xF00) + (regs.eax & 0xFF00000);
    bsp_info.model = ((regs.eax & 0xF0000) << 4) + (regs.eax & 0xF0);
    bsp_info.features[0] = regs.edx;
    bsp_info.features[1] = regs.ecx;
    regs.eax = 0x80000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    bsp_info.highest_function = regs.eax;
    if (bsp_info.highest_function < 0x80000001)
        return;
    regs.eax = 0x80000001;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    bsp_info.features[2] = regs.edx;
    bsp_info.features[3] = regs.ecx;
    if (bsp_info.highest_function < 0x80000004)
        return;
    regs.eax = 0x80000002;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    // Take advantage of the fact that the register struct is packed and
    // aligned, so we can directly copy into the char array
    memcpy(&bsp_info.processor_name[0], &regs, 16);
    regs.eax = 0x80000003;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    memcpy(&bsp_info.processor_name[16], &regs, 16);
    regs.eax = 0x80000004;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    memcpy(&bsp_info.processor_name[32], &regs, 16);
    if (bsp_info.highest_function < 0x80000007)
        return;
    regs.eax = 0x80000007;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    bsp_info.invariant_tsc = (regs.edx >> 8);
}

struct cpuinfo* cpu_get_information(int cpu_num)
{
    if (cpu_num == 0) {
        return &bsp_info;
    }
    return NULL;
}
