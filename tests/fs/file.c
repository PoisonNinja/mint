#include <fs/file.h>
#include <fs/stat.h>
#include <kernel/init.h>
#include <string.h>
#include <tests/tests.h>

static int test_file(void)
{
    START_TEST("File operations");
    struct file* file = file_open("/test", 0, 0);
    ASSERT("Opening non-existent file should fail", !file);
    file = file_open("/test", O_CREAT, S_IFDIR);
    ASSERT("Opening with O_CREAT creates file", file);
    ssize_t string_len = strlen("Hello world!") + 1;
    ASSERT("Writing to file succeeds",
           file_pwrite(file, "Hello world!", string_len) == string_len);
    uint8_t buffer[1024];
    ASSERT("Reading from file succeeds",
           file_pread(file, buffer, string_len) == string_len);
    ASSERT("Data read matches written",
           !memcmp(buffer, (uint8_t*)"Hello world!", string_len));
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_file);
