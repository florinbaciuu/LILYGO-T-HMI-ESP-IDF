/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdlib.h>

void *qa_utils_malloc(size_t size, const char* file, int line)
{
    (void)file;  /* unused */
    (void)line;  /* unused */
    return malloc(size);
}


void *qa_utils_calloc(int count, size_t size, const char* file, int line)
{
    (void)file;  /* unused */
    (void)line;  /* unused */
    return calloc(count, size);
}

void qa_utils_free(void *ptr, const char* file, int line)
{
    (void)file;  /* unused */
    (void)line;  /* unused */
    free(ptr);
}
