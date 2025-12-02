/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdarg.h>
#include "sdkconfig.h"

#if CONFIG_QA_UTILS_LOG_OUTPUT_ESP_LOG
#include "esp_log.h"
#endif


void qa_utils_log_writev(const char *format, ...)
{
    va_list list;
    va_start(list, format);
#if CONFIG_QA_UTILS_LOG_OUTPUT_ESP_LOG
    /* use highest log level */
    esp_log_writev(ESP_LOG_ERROR, NULL, format, list);
#elif CONFIG_QA_UTILS_LOG_OUTPUT_PRINTF
    vprintf(format, list);
#else
#error "error log output method!"
#endif
    va_end(list);
}
