/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <errno.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_console.h"

#include "wifi_cmd.h"
#include "ota_cmd.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if CONFIG_ESP_WIFI_ENABLED || CONFIG_ESP_WIFI_REMOTE_ENABLED || CONFIG_ESP_HOST_WIFI_ENABLED
    /* initialise wifi */
    wifi_cmd_initialize_wifi(NULL);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
#endif

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    repl_config.prompt = "test_app>";
    repl_config.max_history_len = 1;
    repl_config.task_priority = 24;
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    /* Register commands */
    wifi_cmd_register_base_basic();
    wifi_cmd_register_sta_basic();
    ota_cmd_register_ota();
    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
