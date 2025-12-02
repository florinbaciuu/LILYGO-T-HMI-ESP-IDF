/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include <errno.h>
#include "sdkconfig.h"
#include "esp_idf_version.h"

#include "esp_app_format.h"
#include "esp_ota_ops.h"


typedef enum {
    OTA_ACTION_START,
    OTA_ACTION_STATUS,
    OTA_ACTION_NEXT,
    OTA_ACTION_END,
    OTA_ACTION_ABORT,
    OTA_ACTION_RESTORE,
    OTA_ACTION_UNKNOWN,
} ota_cmd_action_t;

extern bool g_ota_upgrading;
extern esp_ota_handle_t g_ota_update_handle;

ota_cmd_action_t ota_cmd_str_to_action(const char* str);
void ota_cmd_show_ota_status(void);
esp_err_t ota_cmd_change_next_boot_partition(void);
esp_err_t ota_cmd_restore_first_boot_partition(void);
