#include <kernel/init.h>
#include <mm/heap.h>
#include <tests/tests.h>

static int test_heap(void)
{
    START_TEST("Heap memory manager");
    void* oldptr = kmalloc(0x1000);
    ASSERT("Memory is allocated", oldptr);
    kfree(oldptr);
    void* ptr = NULL;
    ASSERT("Same-sized memory returned is same as before",
           oldptr == (ptr = kmalloc(0x1000)));
    kfree(ptr);
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_heap);
