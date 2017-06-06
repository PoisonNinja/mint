#include <arch/cpu/registers.h>
#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <lib/list.h>
#include <stdatomic.h>

extern void arch_interrupt_disable(void);
extern void arch_interrupt_enable(void);

_Atomic int interrupt_depth = 0;

struct exception_handler* exception_handlers[EXCEPTIONS_MAX];
struct interrupt_handler* interrupt_handlers[INTERRUPTS_MAX];

void interrupt_disable(void)
{
    if (!atomic_fetch_add(&interrupt_depth, 1))
        arch_interrupt_disable();
}

void interrupt_enable(void)
{
    if (atomic_fetch_sub(&interrupt_depth, 1) == 1)
        arch_interrupt_enable();
}

void exception_handler_register(int exception_number,
                                struct exception_handler* handler)
{
    if (!exception_handlers[exception_number]) {
        INIT_LIST_HEAD(&handler->list);
        exception_handlers[exception_number] = handler;
    } else {
        list_add(&handler->list, &exception_handlers[exception_number]->list);
    }
}

void interrupt_handler_register(int interrupt_number,
                                struct interrupt_handler* handler)
{
    if (!interrupt_handlers[interrupt_number]) {
        INIT_LIST_HEAD(&handler->list);
        interrupt_handlers[interrupt_number] = handler;
    } else {
        list_add(&handler->list, &interrupt_handlers[interrupt_number]->list);
    }
}

extern char* arch_exception_translate(int);
void exception_dispatch(struct registers* regs)
{
    if (exception_handlers[regs->int_no]) {
        if (list_only_one(&exception_handlers[regs->int_no]->list)) {
            if (exception_handlers[regs->int_no]->handler)
                exception_handlers[regs->int_no]->handler(
                    regs, exception_handlers[regs->int_no]->dev_id);
        } else {
            struct exception_handler* handler = NULL;
            list_for_each_entry(handler,
                                &exception_handlers[regs->int_no]->list, list)
            {
                if (handler->handler)
                    handler->handler(regs, handler->dev_id);
            }
        }
    } else {
        printk(INFO, "Unhandled exception %d: %s\n", regs->int_no,
               arch_exception_translate(regs->int_no));
    }
}

void interrupt_dispatch(struct registers* regs)
{
    if (interrupt_handlers[regs->int_no]) {
        if (list_only_one(&interrupt_handlers[regs->int_no]->list)) {
            if (interrupt_handlers[regs->int_no]->handler)
                interrupt_handlers[regs->int_no]->handler(
                    regs, interrupt_handlers[regs->int_no]->dev_id);
        } else {
            struct interrupt_handler* handler = NULL;
            list_for_each_entry(handler,
                                &interrupt_handlers[regs->int_no]->list, list)
            {
                if (handler->handler)
                    handler->handler(regs, handler->dev_id);
            }
        }
    }
    interrupt_controller_ack(regs->int_no);
}
