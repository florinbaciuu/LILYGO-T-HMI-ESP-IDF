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
#include "esp_http_client.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"
#include "esp_console.h"

#include "ota_cmd.h"
#include "ota_common.h"

#if CONFIG_OTA_CMD_USE_ESP_HTTPS_OTA
#include "esp_https_ota.h"
#endif

#ifndef esp_ota_abort // version <= v4.2
#define esp_ota_abort esp_ota_end
#endif

#ifndef APP_TAG
#define APP_TAG "OTA"
#endif

#define CONFIG_EXAMPLE_OTA_RECV_TIMEOUT (3000)

#define OTA_URL_SIZE (256)
#define BUFFSIZE (1024)
#define HASH_LEN (32)
static char s_ota_upgrade_url[OTA_URL_SIZE + 1] = "";
/* update handle: set by esp_ota_begin(), must be freed via esp_ota_end() */
esp_ota_handle_t g_ota_update_handle = 0 ;


typedef struct {
    struct arg_str *action;
    struct arg_str *url;
    struct arg_int *http_timeout;
    struct arg_end *end;
} ota_args_t;
static ota_args_t s_ota_args;

#if !CONFIG_OTA_CMD_USE_ESP_HTTPS_OTA
static char ota_write_data[BUFFSIZE + 1] = { 0 };

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) ota_task_fatal_error(void)
{
    ESP_LOGE(APP_TAG, "@OTA_ERROR! Exiting task due to fatal error...");
    g_ota_upgrading = false;
    (void)vTaskDelete(NULL);
    while (1) {
        ;
    }
}

static void infinite_loop(void)
{
    int i = 0;
    ESP_LOGI(APP_TAG, "When a new firmware is available on the server, press the reset button to download it");
    while (1) {
        ESP_LOGI(APP_TAG, "Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    ESP_LOGD(APP_TAG, "HTTP_EVENT:%d", evt->event_id);
    return ESP_OK;
}

// esp-idf/examples/system/ota/native_ota_example/main/native_ota_example.c
static void simple_ota_http_task(void *pvParameter)
{
    esp_err_t err;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(APP_TAG, "Starting OTA task");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(APP_TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(APP_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(APP_TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = s_ota_upgrade_url,
        // Use simple http server
        // .cert_pem = (char *)server_cert_pem_start,
        // .skip_cert_common_name_check = true;
        .timeout_ms = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(APP_TAG, "Failed to initialise HTTP connection");
        ota_task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(APP_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        ota_task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    ESP_LOGI(APP_TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(APP_TAG, "Error: SSL data read error");
            http_cleanup(client);
            ota_task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(APP_TAG, "New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        ESP_LOGI(APP_TAG, "Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        ESP_LOGI(APP_TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            ESP_LOGW(APP_TAG, "New version is the same as invalid version.");
                            ESP_LOGW(APP_TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(APP_TAG, "The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            infinite_loop();
                        }
                    }
#if !CONFIG_OTA_CMD_SKIP_VERSION_CHECK
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(APP_TAG, "Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);
                        infinite_loop();
                    }
#endif

                    image_header_was_checked = true;
#ifdef OTA_WITH_SEQUENTIAL_WRITES // version >= 4.1
                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &g_ota_update_handle);
#else
                    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &g_ota_update_handle);
#endif
                    if (err != ESP_OK) {
                        ESP_LOGE(APP_TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        esp_ota_abort(g_ota_update_handle);
                        ota_task_fatal_error();
                    }
                    ESP_LOGI(APP_TAG, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(APP_TAG, "received package is not fit len");
                    http_cleanup(client);
                    esp_ota_abort(g_ota_update_handle);
                    ota_task_fatal_error();
                }
            }
            err = esp_ota_write( g_ota_update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                esp_ota_abort(g_ota_update_handle);
                ota_task_fatal_error();
            }
            binary_file_length += data_read;
            ESP_LOGD(APP_TAG, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
            /*
             * As esp_http_client_read never returns negative error code, we rely on
             * `errno` to check for underlying transport connectivity closure if any
             */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(APP_TAG, "Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                ESP_LOGI(APP_TAG, "Connection closed");
                break;
            }
        }
    }
    ESP_LOGI(APP_TAG, "Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) {
        ESP_LOGE(APP_TAG, "Error in receiving complete file");
        http_cleanup(client);
        esp_ota_abort(g_ota_update_handle);
        ota_task_fatal_error();
    }

    err = esp_ota_end(g_ota_update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(APP_TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(APP_TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);
        ota_task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(APP_TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        ota_task_fatal_error();
    }
    ESP_LOGI(APP_TAG, "OTA Succeed. Ready for restarting.");
    vTaskDelete(NULL);
}

#endif /* OTA_CMD_USE_ESP_HTTPS_OTA */


static int cmd_do_ota(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &s_ota_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_ota_args.end, argv[0]);
        return 1;
    }

    ota_cmd_action_t action = ota_cmd_str_to_action(s_ota_args.action->sval[0]);

    switch (action) {
        case OTA_ACTION_STATUS:
            ota_cmd_show_ota_status();
            break;
        case OTA_ACTION_START:
            if (s_ota_args.url->count == 0) {
                ESP_LOGE(APP_TAG, "ota start: no url specified.");
                return 1;
            }
            g_ota_upgrading = true;
            strncpy(s_ota_upgrade_url, s_ota_args.url->sval[0], OTA_URL_SIZE);
#if CONFIG_OTA_CMD_USE_ESP_HTTPS_OTA
            esp_http_client_config_t config = {
                .url = s_ota_upgrade_url,
                // .cert_pem = (char *)server_cert_pem_start,
            };
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
            esp_https_ota_config_t ota_config = {
                .http_config = &config,
            };
            esp_err_t ret = esp_https_ota(&ota_config);
#else
            esp_err_t ret = esp_https_ota(&config);
#endif /* ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0) */
            g_ota_upgrading = false;
            if (ret != ESP_OK) {
                ESP_LOGE(APP_TAG, "esp_https_ota failed: %d", ret);
                return 1;
            }
            ESP_LOGI(APP_TAG, "OTA Succeed. Ready for restarting.");
#else
            xTaskCreate(&simple_ota_http_task, "ota_http_task", 8192, NULL, 5, NULL);
#endif /* CONFIG_OTA_CMD_USE_ESP_HTTPS_OTA */
            break;
        case OTA_ACTION_NEXT:
            ota_cmd_change_next_boot_partition();
            esp_restart();
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
            ESP_LOGE(APP_TAG, "Unknown ota action.");
            break;
    }
    return 0;
}


void ota_cmd_register_ota(void)
{
    s_ota_args.action = arg_str1(NULL, NULL, "<action>", "start,\n\t\tstatus,\n\t\tnext (reboot to next app partition),\n\t\trestore (reboot to first app partition),\n\t\tend (reboot/esp_restart).");
    s_ota_args.url = arg_str0(NULL, NULL, "<url>", "URL of server which hosts the firmware image.");
    s_ota_args.http_timeout = arg_int0(NULL, "http-timeout", "<int>", "http[s] response timeout (ms).");
    s_ota_args.end = arg_end(1);
    const esp_console_cmd_t ota_start = {
        .command = "ota",
        .help = "http[s] OTA upgrade command.",
        .hint = NULL,
        .func = &cmd_do_ota,
        .argtable = &s_ota_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&ota_start) );
}
