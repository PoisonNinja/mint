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

#include <lib/list.h>
#include <types.h>

#define IC_NONE 0
#define IC_INTEL_8259 1

#define IC_STATUS_UNINITIALIZED 0
#define IC_STATUS_DISABLED 1
#define IC_STATUS_ENABLED 2

struct interrupt_controller {
    const char* name;
    uint32_t identifier;  // Should match IC_whatever
    uint32_t status;
    int (*init)(void);
    int (*enable)(void);
    int (*disable)(void);
    int (*mask)(int);
    int (*unmask)(int);
    int (*ack)(int);
    struct list_element list;
};

extern int interrupt_controller_register(
    struct interrupt_controller* controller);
extern int interrupt_controller_unregister(
    struct interrupt_controller* controller);
extern int interrupt_controller_set(uint32_t);

extern int interrupt_controller_enable(void);
extern int interrupt_controller_disable(void);
extern int interrupt_controller_mask(int);
extern int interrupt_controller_unmask(int);
extern int interrupt_controller_ack(int);
