/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#include "esp_sleep.h"

static const char* TAG = SYS_CMD_TAG;

typedef struct {
    struct arg_dbl *time_ms;
    struct arg_dbl *time_us;
    struct arg_end *end;
} dsleep_args_t;
static dsleep_args_t dsleep_args;


static int cmd_do_dsleep(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &dsleep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, dsleep_args.end, argv[0]);
        return 1;
    }

    uint64_t dsleep_time_us = 0;
    if (dsleep_args.time_ms->count > 0) {
        dsleep_time_us += dsleep_args.time_ms->dval[0] * 1000;
    }
    if (dsleep_args.time_us->count > 0) {
        dsleep_time_us += dsleep_args.time_us->dval[0];
    }
    ESP_LOGI(TAG, "DEEP_SLEEP_START,%lluus", dsleep_time_us);
    esp_deep_sleep(dsleep_time_us);
    return 0;
}

void sys_cmd_register_dsleep(void)
{
    dsleep_args.time_ms = arg_dbl0("t", "ms", "<time_ms>", "deep sleep time in ms");
    dsleep_args.time_us = arg_dbl0(NULL, "us", "<time_us>", "deep sleep time in us");
    dsleep_args.end = arg_end(2);

    const esp_console_cmd_t dsleep_cmd = {
        .command = "dsleep",
        .help = "esp_deep_sleep with given time",
        .hint = NULL,
        .func = &cmd_do_dsleep,
        .argtable = &dsleep_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&dsleep_cmd) );

}
