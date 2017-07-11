#include <cpu/exception.h>
#include <cpu/interrupt.h>
#include <cpu/power.h>
#include <kernel.h>
#include <lib/list.h>

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
void exception_dispatch(struct registers* regs)
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
