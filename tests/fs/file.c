#include <fs/file.h>
#include <fs/stat.h>
#include <kernel/init.h>
#include <tests/tests.h>

static int test_file(void)
{
    START_TEST("File operations");
    struct file* file = file_open("/test", 0, 0);
    ASSERT("Opening non-existent file should fail", !file);
    file = file_open("/test", O_CREAT, S_IFDIR);
    ASSERT("Opening with O_CREAT creates file", file);
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_file);
