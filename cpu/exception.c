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

#include <cpu/exception.h>
#include <cpu/interrupt.h>
#include <cpu/power.h>
#include <kernel.h>

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
            (element)->prev = (element);    \
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

static struct exception_handler* exception_handlers[EXCEPTIONS_MAX];

void exception_handler_register(int exception_number,
                                struct exception_handler* handler)
{
    LIST_APPEND(exception_handlers[exception_number], handler);
}

void exception_handler_unregister(int exception_number,
                                  struct exception_handler* handler)
{
    LIST_REMOVE(exception_handlers[exception_number], handler);
}

extern char* arch_exception_translate(int);
void exception_dispatch(struct interrupt_ctx* regs)
{
    if (exception_handlers[regs->int_no]) {
        struct exception_handler* handler = NULL;
        LIST_FOR_EACH(exception_handlers[regs->int_no], handler)
        {
            if (handler->handler)
                handler->handler(regs, handler->dev_id);
        }
    } else {
        printk(ERROR, "Unhandled exception %d: %s\n", regs->int_no,
               arch_exception_translate(regs->int_no));
        interrupt_disable();
        for (;;)
            cpu_halt();
    }
}
