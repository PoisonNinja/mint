#pragma once

#include <arch/cpu/registers.h>
#include <tm/process.h>

struct thread {
    struct process* process;
    struct registers* registers;
};
