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

#define EXCEPTIONS_MAX 64
#define INTERRUPTS_MAX 256

#include <arch/cpu/registers.h>
#include <lib/list.h>

struct registers;

typedef int (*exception_handler_t)(struct registers *, void *);
typedef int (*irq_handler_t)(struct registers *, void *);

struct exception_handler {
    exception_handler_t handler;
    const char *dev_name;
    void *dev_id;
    struct list_head list;
};

struct interrupt_handler {
    irq_handler_t handler;
    const char *dev_name;
    void *dev_id;
    struct list_head list;
};

extern void exception_handler_register(int exception_number,
                                       struct exception_handler *handler);
extern void exception_handler_unregister(int exception_number,
                                         struct exception_handler *handler);
extern void interrupt_handler_register(int interrupt_number,
                                       struct interrupt_handler *handler);
extern void interrupt_handler_unregister(int interrupt_number,
                                         struct interrupt_handler *handler);

extern void interrupt_disable(void);
extern void interrupt_enable(void);
