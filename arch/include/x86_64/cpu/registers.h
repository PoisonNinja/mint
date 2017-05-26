#pragma once

#include <types.h>

struct registers {
    uint64_t r15, r14, r13, r12, rbp, rbx, r11, r10, r9, r8, rax, rcx, rdx, rsi,
        rdi;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, userrsp, ss;
} __attribute__((packed));
