/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "sdkconfig.h"

#include "esp_idf_version.h"
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_system.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#define CONFIG_ESP_WIFI_ENABLED (1)
#elif ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)
#define CONFIG_ESP_WIFI_ENABLED (CONFIG_ESP32_WIFI_ENABLED)
#endif

static const char* TAG = SYS_CMD_TAG;

typedef struct {
    struct arg_str *mac;
    struct arg_end *end;
} mac_args_t;
static mac_args_t mac_args;


#if !CONFIG_IDF_TARGET_LINUX
static esp_err_t str2mac(const char *str, uint8_t *mac_addr)
{
    unsigned int mac_tmp[6];
    if (6 != sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x%*c",
                    &mac_tmp[0], &mac_tmp[1], &mac_tmp[2],
                    &mac_tmp[3], &mac_tmp[4], &mac_tmp[5])) {
        return ESP_FAIL;
    }
    for (int i = 0; i < 6; i++) {
        mac_addr[i] = (uint8_t)mac_tmp[i];
    }
    return ESP_OK;
}
#endif

static int cmd_do_mac(int argc, char **argv)
{
#if CONFIG_IDF_TARGET_LINUX
    ESP_LOGE(TAG, "get/set mac on linux target is not supported");
#else
    int nerrors = arg_parse(argc, argv, (void **) &mac_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mac_args.end, argv[0]);
        return 1;
    }
    uint8_t mac[6] = {0};
    if (mac_args.mac->count > 0) {
        /* Set MAC */
        const char *mac_str = mac_args.mac->sval[0];
        if (str2mac(mac_str, mac) != ESP_OK) {
            ESP_LOGE(TAG, "invalid mac address.");
        }
        ESP_ERROR_CHECK(esp_base_mac_addr_set(mac));
        ESP_LOGI(TAG, "BASEMAC_SET:"MACSTR, MAC2STR(mac));
    } else {
        /* Get MAC */
        if (esp_base_mac_addr_get(mac) == ESP_ERR_INVALID_MAC) {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)
            /* idf <= 5.0 */
            ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
#endif
        }
        ESP_LOGI(TAG, "BASEMAC:"MACSTR, MAC2STR(mac));
        /* use esp_read_mac to get wifi/bt/i154 mac address, for script auto-get mac */
        /* NOTE: this may not always be same with esp_wifi_read_mac/esp_bt_dev_get_address */
#if CONFIG_ESP_WIFI_ENABLED
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
        ESP_LOGI(TAG, "WIFI_STAMAC:"MACSTR, MAC2STR(mac));
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP));
        ESP_LOGI(TAG, "WIFI_APMAC:"MACSTR, MAC2STR(mac));
#endif
#if CONFIG_BT_ENABLED
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_BT));
        ESP_LOGI(TAG, "BTMAC:"MACSTR, MAC2STR(mac));
#endif
#if CONFIG_IEEE802154_ENABLED
#define ESP_MAC_ADDRESS_LEN (8)
        uint8_t eui64[ESP_MAC_ADDRESS_LEN] = {0};
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_IEEE802154));
        ESP_LOGI(TAG, "I154MAC:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                 eui64[0], eui64[1], eui64[2], eui64[3], eui64[4], eui64[5], eui64[6], eui64[7]);
#endif
    }
#endif /* CONFIG_IDF_TARGET_LINUX */
    return 0;
}

void sys_cmd_register_mac(void)
{
    mac_args.mac = arg_str0(NULL, NULL, "<mac>", "set base mac address.");
    mac_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "mac",
        .help = "Get/Set base mac address.",
        .hint = NULL,
        .func = &cmd_do_mac,
        .argtable = &mac_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
