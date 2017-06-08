#pragma once

#include <types.h>

struct cpuinfo {
    uint8_t stepping;
    uint8_t type;
    uint8_t family;
    uint8_t model;
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
