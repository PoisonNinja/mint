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

#pragma once

#include <types.h>

// EAX = 1, EDX
#define X86_CPU_HAS_FPU(x) ((x->features[0]) & (1 << 0))
#define X86_CPU_HAS_VME(x) ((x->features[0]) & (1 << 1))
#define X86_CPU_HAS_DE(x) ((x->features[0]) & (1 << 2))
#define X86_CPU_HAS_PSE(x) ((x->features[0]) & (1 << 3))
#define X86_CPU_HAS_TSC(x) ((x->features[0]) & (1 << 4))
#define X86_CPU_HAS_MSR(x) ((x->features[0]) & (1 << 5))
#define X86_CPU_HAS_PAE(x) ((x->features[0]) & (1 << 6))
#define X86_CPU_HAS_MCE(x) ((x->features[0]) & (1 << 7))
#define X86_CPU_HAS_CX8(x) ((x->features[0]) & (1 << 8))
#define X86_CPU_HAS_APIC(x) ((x->features[0]) & (1 << 9))
#define X86_CPU_HAS_SEP(x) ((x->features[0]) & (1 << 11))
#define X86_CPU_HAS_MTRR(x) ((x->features[0]) & (1 << 12))
#define X86_CPU_HAS_PGE(x) ((x->features[0]) & (1 << 13))
#define X86_CPU_HAS_MCA(x) ((x->features[0]) & (1 << 14))
#define X86_CPU_HAS_CMOV(x) ((x->features[0]) & (1 << 15))
#define X86_CPU_HAS_PAT(x) ((x->features[0]) & (1 << 16))
#define X86_CPU_HAS_PSE36(x) ((x->features[0]) & (1 << 17))
#define X86_CPU_HAS_PSN(x) ((x->features[0]) & (1 << 18))
#define X86_CPU_HAS_CFLUSH(x) ((x->features[0]) & (1 << 19))
#define X86_CPU_HAS_DS(x) ((x->features[0]) & (1 << 21))
#define X86_CPU_HAS_ACPI(x) ((x->features[0]) & (1 << 22))
#define X86_CPU_HAS_MMX(x) ((x->features[0]) & (1 << 23))
#define X86_CPU_HAS_FXSR(x) ((x->features[0]) & (1 << 24))
#define X86_CPU_HAS_SSE(x) ((x->features[0]) & (1 << 25))
#define X86_CPU_HAS_SSE2(x) ((x->features[0]) & (1 << 26))
#define X86_CPU_HAS_SS(x) ((x->features[0]) & (1 << 27))
#define X86_CPU_HAS_HTT(x) ((x->features[0]) & (1 << 28))
#define X86_CPU_HAS_TM(x) ((x->features[0]) & (1 << 29))
#define X86_CPU_HAS_PBE(x) ((x->features[0]) & (1 << 31))

struct cpuinfo {
    uint8_t stepping;
    uint8_t type;
    uint8_t family;
    uint8_t model;
    uint8_t invariant_tsc;
    uint32_t features[4];
    uint32_t highest_function;
    char signature[13];
    char processor_name[48];
};

static inline void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx,
                         uint32_t* edx)
{
    __asm__("cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "0"(*eax), "2"(*ecx)
            : "memory");
}

extern void cpu_print_information(struct cpuinfo* cpu);
extern void cpu_initialize_information(void);
struct cpuinfo* cpu_get_information(int cpu_num);
