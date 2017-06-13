#pragma once

#include <lib/list.h>
#include <types.h>

struct buddy_order {
    struct list_head free;
    uint8_t* bitmap;
};

struct buddy {
    uint8_t
        max_order;  // Unlikely the order (2^n) will ever go beyond 256, because
                    // that is 1.157920892E77 bytes of memory
    struct buddy_order* orders;
};
