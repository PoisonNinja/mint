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
    struct interrupt_controller *next, *prev;
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
