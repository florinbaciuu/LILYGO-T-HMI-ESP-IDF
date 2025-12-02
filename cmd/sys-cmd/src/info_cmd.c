/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_idf_version.h"
#include "sys_cmd.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_chip_info.h"
#endif

static const char* TAG = SYS_CMD_TAG;

typedef struct {
    struct arg_str *type;
    struct arg_end *end;
} info_args_t;
static info_args_t info_args;

typedef enum {
    INFO_TYPE_ALL = 0,
    INFO_TYPE_VERSION,
    INFO_TYPE_CHIP,
    INFO_TYPE_CONFIG,
    INFO_TYPE_TASK,
} info_type_t;


void app_print_info_chip()
{
    /* Print chip information */
    // ESP_LOGI(TAG, "Info Chip:");
    ESP_LOGI(TAG, "CHIPTYPE:%s", CONFIG_IDF_TARGET);
    esp_chip_info_t chip_info = {0};
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, " CHIPVER:%d", chip_info.revision);
    ESP_LOGI(TAG, " CPUCores:%d", chip_info.cores);
    ESP_LOGI(TAG, " Features:%x-%s%s%s%s", chip_info.features,
             chip_info.features & CHIP_FEATURE_WIFI_BGN ? "802.11bgn," : "",
             chip_info.features & CHIP_FEATURE_BLE ? "BLE," : "",
             chip_info.features & CHIP_FEATURE_BT ? "BT," : "",
             chip_info.features & CHIP_FEATURE_EMB_FLASH ? "Embedded-Flash" : "External-Flash"
            );

    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(TAG, " FlashSize:%uMB", flash_size / (1024 * 1024));
}

void app_print_info_config()
{
    ESP_LOGI(TAG, "Info Config:");
#if CONFIG_PM_ENABLE
    ESP_LOGI(TAG, " PM_ENABLE:1");
#if CONFIG_PM_DFS_INIT_AUTO
    ESP_LOGI(TAG, " PM_DFS_INIT_AUTO:1");
#endif /* CONFIG_PM_DFS_INIT_AUTO */
#endif /* CONFIG_PM_ENABLE */

}

void app_print_info_task()
{
#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    uint8_t *CPU_RunInfo = malloc(2000);
    vTaskGetRunTimeStats((char *)CPU_RunInfo);
    ESP_LOGI(TAG, "Info Task:");
    ESP_LOGI(TAG, " task_name       TCNT         ratio\n");
    ESP_LOGI(TAG, " %s", CPU_RunInfo);
    free(CPU_RunInfo);
#else
    ESP_LOGI(TAG, "Can not show task info, please Enable FREERTOS_GENERATE_RUN_TIME_STATS.");
#endif
}

int app_print_version()
{
    ESP_LOGI(TAG, "IDF describe: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "SDKVER:v%d.%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
    return 0;
}

static int cmd_do_print_info(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &info_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, info_args.end, argv[0]);
        return 1;
    }

    info_type_t type = INFO_TYPE_ALL;
    if (info_args.type->count > 0) {
        const char *pstr = info_args.type->sval[0];
        if (strcmp(pstr, "all") == 0) {
            type = INFO_TYPE_ALL;
        } else if (strcmp(pstr, "version") == 0) {
            type = INFO_TYPE_VERSION;
        } else if (strcmp(pstr, "chip") == 0) {
            type = INFO_TYPE_CHIP;
        } else if (strcmp(pstr, "config") == 0) {
            type = INFO_TYPE_CONFIG;
        } else if (strcmp(pstr, "task") == 0) {
            type = INFO_TYPE_TASK;
        }
    }
    switch (type) {
    case INFO_TYPE_ALL:
    case INFO_TYPE_VERSION:
        app_print_version();
        if (type != INFO_TYPE_ALL) {
            break;
        }
    /* fall through */
    case INFO_TYPE_CHIP:
        app_print_info_chip();
        if (type != INFO_TYPE_ALL) {
            break;
        }
    /* fall through */
    case INFO_TYPE_CONFIG:
        app_print_info_config();
        if (type != INFO_TYPE_ALL) {
            break;
        }
    /* fall through */
    case INFO_TYPE_TASK:
        app_print_info_task();
        break;
    default:
        ESP_LOGW(TAG, "Unknown type!");
        return 1;
    }
    ESP_LOGI(TAG, "INFO,DONE.");
    return 0;
}

void sys_cmd_register_info(void)
{
    info_args.type = arg_str0(NULL, NULL, "<type>", "all/version/chip/config/task. default: all");
    info_args.end = arg_end(1);
    const esp_console_cmd_t info_cmd = {
        .command = "info",
        .help = "Query Info.",
        .hint = NULL,
        .func = &cmd_do_print_info,
        .argtable = &info_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&info_cmd));
}
