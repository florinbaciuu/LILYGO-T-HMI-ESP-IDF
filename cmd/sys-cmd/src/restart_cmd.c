/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_log.h"
#include "esp_system.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"

static int cmd_do_restart(int argc, char **argv)
{
    esp_restart();
    return 0;
}


void sys_cmd_register_restart(void)
{
    const esp_console_cmd_t restart_cmd = {
        .command = "restart",
        .help = "Do esp restart",
        .hint = NULL,
        .func = &cmd_do_restart,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&restart_cmd));
}
