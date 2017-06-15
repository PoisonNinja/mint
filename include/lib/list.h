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

#include <kernel.h>
#include <types.h>

#define LIST_FOR_EACH(head, element)    \
    for ((element) = (head); (element); \
         (element) = (((element)->next == (head)) ? NULL : (element)->next))

#define LIST_APPEND(head, element)          \
    do {                                    \
        if (!head) {                        \
            (element)->next = (element);    \
            (element)->prev = (element);    \
            (head) = (element);             \
        } else {                            \
            (element)->next = (head);       \
            (element)->prev = (head)->prev; \
            (head)->prev->next = (element); \
            (head)->prev = (element);       \
        }                                   \
    } while (0)

#define LIST_PREPEND(head, element)         \
    do {                                    \
        if (!head) {                        \
            (element)->next = (element);    \
            (element)->prev = (element)     \
        } else {                            \
            (element)->next = (head);       \
            (element)->prev = (head)->prev; \
            (head)->prev->next = (element); \
            (head)->prev = (element);       \
        }                                   \
        (head) = (element);                 \
    } while (0)

#define LIST_REMOVE(head, element)                                 \
    do {                                                           \
        if ((head) == (element) && (element)->next == (element)) { \
            (head) = NULL;                                         \
        } else {                                                   \
            (element)->next->prev = (element)->prev;               \
            (element)->prev->next = (element)->next;               \
            (element)->next = (element);                           \
            (element)->prev = (element);                           \
        }                                                          \
    } while (0)
