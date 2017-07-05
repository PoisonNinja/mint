#include <lib/stack.h>

/*
 * To prevent overruns, since NULL could actually be a valid value, especially
 * when used with a buddy allocator (memory starts at 0x0)
 */
static struct stack_item sentry;

void stack_init(struct stack* stack)
{
    stack->size = 0;
    stack->top = &sentry;
}

size_t stack_push(struct stack* stack, struct stack_item* item)
{
    stack->size++;
    stack->top->next = item;
    item->prev = stack->top;
    item->next = &sentry;
    stack->top = item;
    return stack->size;
}

void* stack_pop(struct stack* stack)
{
    struct stack_item* item = stack->top;
    if (item == &sentry)
        return NULL;
    if (item->prev == &sentry)
        stack->top = &sentry;
    else {
        stack->top->prev->next = &sentry;
        stack->top = stack->top->prev;
    }
    stack->size--;
    return item->data;
}
