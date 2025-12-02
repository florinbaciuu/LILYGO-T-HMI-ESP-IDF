/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <inttypes.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "soc/soc.h"

static const char* TAG = SYS_CMD_TAG;

typedef struct {
    struct arg_dbl *reg_addr;
    struct arg_dbl *value;
    struct arg_end *end;
} reg_args_t;
static reg_args_t s_reg_args;


static int cmd_reg_config(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &s_reg_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_reg_args.end, argv[0]);
        return 1;
    }

    uintptr_t reg_addr = (uintptr_t) s_reg_args.reg_addr->dval[0];
    uint32_t value = 0;
    if (s_reg_args.value->count > 0) {
        value = (uint32_t) s_reg_args.value->dval[0];
        REG_WRITE(reg_addr, value);
        ESP_LOGI(TAG, "REG_WRITE:0x%08"PRIx32",0x%08"PRIx32, reg_addr, value);
    }
    value = REG_READ(reg_addr);
    ESP_LOGI(TAG, "REG_READ:0x%08"PRIx32",0x%08"PRIx32, reg_addr, value);
    return 0;
}

void sys_cmd_register_reg(void)
{
    s_reg_args.reg_addr = arg_dbl1(NULL, NULL, "<REG_ADDR>", "the address of the register to read or write");
    s_reg_args.value = arg_dbl0("v", "value", "<REG_VALUE>", "the value to write to the register");
    s_reg_args.end = arg_end(2);

    const esp_console_cmd_t reg_cmd = {
        .command = "reg",
        .help = "Read or write register.",
        .hint = NULL,
        .func = &cmd_reg_config,
        .argtable = &s_reg_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&reg_cmd) );
}
