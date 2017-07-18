#include <kernel.h>
#include <lib/list.h>
#include <mm/heap.h>
#include <string.h>
#include <tests/tests.h>

struct {
    uint8_t pass, fail;
} current_test;

void pass()
{
    current_test.pass++;
    printk(CONTINUE, "\e[32mPASS\n");
}

void fail()
{
    current_test.fail++;
    printk(CONTINUE, "\e[31mFAIL\n");
}

void test_start(const char* msg)
{
    memset(&current_test, 0, sizeof(current_test));
    printk(DEBUG, "Testing: %s\n", msg);
}

void test_print(const char* msg)
{
    printk(DEBUG, "    %s ... ", msg);
}

void test_finish(void)
{
    printk(DEBUG, "Test results: %u passed, %u failed, %u total\n",
           current_test.pass, current_test.fail,
           current_test.pass + current_test.fail);
}
