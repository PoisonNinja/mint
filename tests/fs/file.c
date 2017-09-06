/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <fs/file.h>
#include <fs/stat.h>
#include <kernel/init.h>
#include <string.h>
#include <tests/tests.h>

static int test_file(void)
{
    START_TEST("File operations");
    struct file *file = file_open("/test", 0, 0);
    ASSERT("Opening non-existent file should fail", !file);
    file = file_open("/test", O_CREAT, S_IFDIR);
    ASSERT("Opening with O_CREAT creates file", file);
    file = file_open("/test", 0, 0);
    ASSERT("Opening file again without O_CREAT succeeds", file);
    ssize_t string_len = strlen("Hello world!") + 1;
    ASSERT(
        "Writing to file succeeds",
        file_pwrite(file, (uint8_t *)"Hello world!", string_len) == string_len);
    uint8_t buffer[1024];
    ASSERT("Reading from file succeeds",
           file_pread(file, buffer, string_len) == string_len);
    ASSERT("Data read matches written",
           !memcmp(buffer, (uint8_t *)"Hello world!", string_len));
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_file);
