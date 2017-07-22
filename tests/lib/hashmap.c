#include <kernel/init.h>
#include <lib/hashmap.h>
#include <tests/tests.h>

static int test_hashmap(void)
{
    START_TEST("Hashmap library");
    struct hashmap* hashmap = hashmap_create(0x100);
    ASSERT("Hashmap allocated", hashmap);
    size_t idx = 0;
    ASSERT("Hashmap can set data",
           (idx = hashmap_set(hashmap, "test", 4, (void*)0xDEADBEEF)));
    void* data = hashmap_get(hashmap, "test", 4);
    ASSERT("Hashmap can retrieve data", data);
    ASSERT("Hashmap data is same as original", data == (void*)0xDEADBEEF);
    ASSERT("Hashmap has different index for new value",
           hashmap_set(hashmap, "blah", 4, (void*)0xDEADBEEF) != idx);
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_hashmap);
