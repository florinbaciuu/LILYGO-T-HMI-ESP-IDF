/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * include: restart command
 */
void sys_cmd_register_restart(void);

/**
 * include: heap command
 */
void sys_cmd_register_heap(void);

/**
 * include: mac command
 */
void sys_cmd_register_mac(void);

/**
 * include: info command
 */
void sys_cmd_register_info(void);

/**
 * include: reg command
 */
void sys_cmd_register_reg(void);

#if CONFIG_SYS_CMD_SUPPORT_NVS
/**
 * include: nvs command
 */
void sys_cmd_register_nvs(void);
#endif

#if CONFIG_SYS_CMD_SUPPORT_DSLEEP
/**
 * include command: dsleep
 */
void sys_cmd_register_dsleep(void);
#endif

/**
 * include: restart/heap/mac/info/etc commands
 */
void sys_cmd_register_all(void);

/* Deprecated */
#define app_register_restart_command sys_cmd_register_restart
#define app_register_heap_command sys_cmd_register_heap
#define app_register_mac_command sys_cmd_register_mac
#define app_register_info_command sys_cmd_register_info
#define app_register_nvs_command sys_cmd_register_nvs
#define app_register_system_commands sys_cmd_register_all

#ifdef __cplusplus
}
#endif
