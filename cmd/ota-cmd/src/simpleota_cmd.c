/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include <errno.h>
#include "sdkconfig.h"
#include "esp_idf_version.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"
#include "esp_console.h"


#include "ota_cmd.h"
#include "ota_common.h"

#ifndef esp_ota_abort // version <= v4.2
#define esp_ota_abort esp_ota_end
#endif

#ifndef TAG
#define TAG "OTA"
#endif

#define BUFFER_SIZE (1024)

typedef struct {
    struct arg_str *action;
    struct arg_lit *no_reboot;
    struct arg_end *end;
} simpleota_args_t;
static simpleota_args_t s_simple_ota_args;


static void simple_ota_task(void *pvParameter)
{
    esp_err_t err = ESP_OK;
    size_t offset = 0;
    char* buffer = NULL;

    esp_ota_handle_t g_ota_update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (boot_partition != running_partition) {
        ESP_LOGW(TAG, "boot_partition is not running_partition");
    }

    // do {
        update_partition = esp_ota_get_next_update_partition(NULL);
        if (update_partition == NULL) {
            ESP_LOGE(TAG, "failed to get next update partition");
            err = ESP_FAIL;
            goto _err_end;
        }
        ESP_LOGI(TAG, "NextPartition,0x%08x,type:0x%x", update_partition->address, update_partition->subtype);
        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &g_ota_update_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ota_begin failed");
            goto _err_end;
        }
        ESP_LOGI(TAG, "OTABegin,OK");

    buffer = (char*)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "@EW, failed to malloc buffer, size: %d", BUFFER_SIZE);
        err = ESP_ERR_NO_MEM;
        goto _err_end;
    }
    /* Read from running_partition and write to update_partition */
    while (offset < running_partition->size) {
        size_t chunk_size = BUFFER_SIZE;
        // Adjust chunk size if remaining data is smaller than the buffer
        if (offset + chunk_size > running_partition->size) {
            chunk_size = running_partition->size - offset;
        }

        // Read from source partition
        err = esp_partition_read(running_partition, offset, buffer, chunk_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "failed to read running_partition: %d", err);
            goto _err_end;
        }

        /* use esp_ota_write rather than esp_partition_write */
        err = esp_ota_write(g_ota_update_handle, buffer, chunk_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write ota: %d", err);
            goto _err_end;
        }

        offset += chunk_size;
        ESP_LOGD(TAG, "write %d bytes (total: %d/%d)", chunk_size, offset, running_partition->size);
    }

    /* ota end */
    err = esp_ota_end(g_ota_update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed, %d", err);
        goto _err_end;
    }
    g_ota_update_handle = 0;
    /* set new boot partition */
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "set boot partition failed, %d", err);
        goto _err_end;
    }
    g_ota_upgrading = false;
    ESP_LOGI(TAG, "OTA Succeed. Ready for restarting.");
    vTaskDelete(NULL);

_err_end:
    if (buffer != NULL) free(buffer);
    if (g_ota_update_handle != 0) esp_ota_abort(g_ota_update_handle);

    ESP_LOGI(TAG, "SimpleOTA,FAIL,%d", err);
    g_ota_upgrading = false;
    vTaskDelete(NULL);
}


static int cmd_do_simpleota(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &s_simple_ota_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_simple_ota_args.end, argv[0]);
        return 1;
    }

    ota_cmd_action_t action = ota_cmd_str_to_action(s_simple_ota_args.action->sval[0]);

    switch (action) {
        case OTA_ACTION_STATUS:
            ota_cmd_show_ota_status();
            break;
        case OTA_ACTION_START:
            if (g_ota_upgrading == true) {
                ESP_LOGE(TAG, "ota_upgrading, can not start new ota task!");
                break;
            }
            g_ota_upgrading = true;
            xTaskCreate(&simple_ota_task, "simpleota_task", 8192, NULL, 5, NULL);
            break;
        case OTA_ACTION_NEXT:
            ota_cmd_change_next_boot_partition();
            if (s_simple_ota_args.no_reboot->count == 0) {
                esp_restart();
            }
            break;
        case OTA_ACTION_END:
            /* Same with "restart", just in case user application didn't register "restart" */
            esp_restart();
            break;
        case OTA_ACTION_ABORT:
            esp_ota_abort(g_ota_update_handle);
            vTaskDelay(1);
            g_ota_upgrading = false;
            break;
        case OTA_ACTION_RESTORE:
            ota_cmd_restore_first_boot_partition();
            esp_restart();
            break;
        default:
            ESP_LOGE(TAG, "Unknown ota action.");
            break;
    }
    return 0;
}

void ota_cmd_register_simpleota(void)
{
    s_simple_ota_args.action = arg_str1(NULL, NULL, "<action>", "start,status,next,restore,end. (same with ota cmd)");
    s_simple_ota_args.no_reboot = arg_lit0(NULL, "no-reboot", "do not restart after 'simpleota next.'");
    s_simple_ota_args.end = arg_end(1);
    const esp_console_cmd_t ota_start = {
        .command = "simpleota",
        .help = "simple OTA cmd, upgrade from local storage.",
        .hint = NULL,
        .func = &cmd_do_simpleota,
        .argtable = &s_simple_ota_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&ota_start) );
}
