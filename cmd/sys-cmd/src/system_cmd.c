/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "sdkconfig.h"
#include "sys_cmd.h"

void sys_cmd_register_all(void)
{
    sys_cmd_register_restart();
    sys_cmd_register_heap();
    sys_cmd_register_info();
    sys_cmd_register_mac();
    sys_cmd_register_reg();
#if CONFIG_SYS_CMD_SUPPORT_NVS
    sys_cmd_register_nvs();
#endif
#if CONFIG_SYS_CMD_SUPPORT_DSLEEP
    sys_cmd_register_dsleep();
#endif
}
