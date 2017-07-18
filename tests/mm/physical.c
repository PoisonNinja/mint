#include <arch/mm/mmap.h>
#include <kernel/init.h>
#include <mm/physical.h>
#include <tests/tests.h>

static int test_physical(void)
{
    START_TEST("Physical memory manager");
    void* oldptr = physical_alloc(0x1000, 0);
    ASSERT("Physical memory is allocated", oldptr);
    physical_free(oldptr, 0x1000);
    void* ptr = NULL;
    ASSERT("Physical memory returned is same as before",
           oldptr == (ptr = physical_alloc(0x1000, 0)));
    physical_free(ptr, 0x1000);
    ASSERT("Physical memory is DMA-capable with PHYS_DMA",
           (addr_t)(ptr = physical_alloc(0x1000, PHYS_DMA)) < DMA_MAX);
    physical_free(ptr, 0x1000);
    ASSERT("Physical memory is normal without PHYS_DMA",
           (addr_t)(ptr = physical_alloc(0x1000, 0)) >= DMA_MAX);
    physical_free(ptr, 0x1000);
    ASSERT("Large physical allocations return NULL",
           physical_alloc(0xFFFFFFFFFFFFFFFF, 0) == NULL);
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_physical);
