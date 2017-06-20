#pragma once

#include <types.h>

struct stack_item {
    void* data;
    struct stack_item *next, *prev;
};

struct stack {
    size_t size;
    struct stack_item* top;
};

extern void stack_init(struct stack* stack);
extern size_t stack_push(struct stack* stack, struct stack_item* item);
extern void* stack_pop(struct stack* stack);
