#include <kernel/console.h>
#include <string.h>

static struct list_head console_list = LIST_HEAD_INIT(console_list);

void console_register(struct console *console)
{
    list_add(&console->list, &console_list);
}

void console_write(const char *message, size_t length)
{
    struct console *current;
    list_for_each_entry(current, &console_list, list)
    {
        current->write(message, length);
    }
}
