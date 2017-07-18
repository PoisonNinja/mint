#pragma once

#include <types.h>

extern void pass();
extern void fail();

extern void test_start(const char* msg);
extern void test_print(const char* msg);
extern void test_finish(void);

#define START_TEST(msg)  \
    do {                 \
        test_start(msg); \
    } while (0)

#define FINISH_TEST()  \
    do {               \
        test_finish(); \
    } while (0)

/*
 * ASSERT: Assert a statement
 *
 * Usage: The statement must evaluate to true, otherwise it will cause an error
 * Basically, think of it like if ((x) == true) pass(); else fail();
 *
 * Examples: ASSERT(1 == 1); // Pass
 *           ASSERT(1 == 2); // Fail
 */
#define ASSERT(msg, x)   \
    do {                 \
        test_print(msg); \
        if (!(x))        \
            fail();      \
        else             \
            pass();      \
    } while (0)
