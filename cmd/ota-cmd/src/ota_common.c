/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_log.h"
#include "ota_common.h"

#define TAG "OTA"

#define HASH_LEN (32)

bool g_ota_upgrading = false;


ota_cmd_action_t ota_cmd_str_to_action(const char* str)
{
    if (!strcmp(str, "start")) {
        return OTA_ACTION_START;
    } else if(!strcmp(str, "status")) {
        return OTA_ACTION_STATUS;
    } else if(!strcmp(str, "next")) {
        return OTA_ACTION_NEXT;
    } else if(!strcmp(str, "end")) {
        return OTA_ACTION_END;
    } else if(!strcmp(str, "abort")) {
        return OTA_ACTION_ABORT;
    } else if(!strcmp(str, "restore")) {
        return OTA_ACTION_RESTORE;
    } else {
        return OTA_ACTION_UNKNOWN;
    }
}

static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s %s", label, hash_print);
}

void ota_cmd_show_ota_status()
{
    if (g_ota_upgrading) {
        ESP_LOGI(TAG, "OTA upgrading ...");
    }

    const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    ESP_LOGI(TAG, "Boot Partition address: 0x%08x", boot_partition->address);
    ESP_LOGI(TAG, "Running Partition address: 0x%08x", running_partition->address);

    uint8_t sha_256[HASH_LEN] = { 0 };
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
    esp_partition_t partition;
    // get sha256 digest for bootloader
    partition.address   = CONFIG_BOOTLOADER_OFFSET_IN_FLASH;
    partition.size      = CONFIG_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");
#endif
    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

esp_err_t ota_cmd_change_next_boot_partition()
{
    esp_err_t ret = ESP_OK;
    const esp_partition_t *update_partition = NULL;
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "failed to get next update partition");
        ESP_LOGI(TAG, "OTA_NEXT,FAIL");
        return ESP_FAIL;
    }
    ret = esp_ota_set_boot_partition(update_partition);
    ESP_LOGI(TAG, "OTA_SET_BOOT_PARTITION:0x%08x,%s,%d", update_partition->address, ret == ESP_OK?"OK":"FAIL", ret);
    /* do not reboot in this function */
    return ret;
}

esp_err_t ota_cmd_restore_first_boot_partition()
{
    esp_err_t ret = ESP_OK;
    const esp_partition_t *restore_partition = NULL;
    restore_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (restore_partition == NULL) {
        ESP_LOGE(TAG, "failed to get first partition");
        ESP_LOGI(TAG, "OTA_RESTORE,FAIL");
        return ESP_FAIL;
    }
    ret = esp_ota_set_boot_partition(restore_partition);
    ESP_LOGD(TAG, "set boot partition to 0x%08x", restore_partition->address);
    ESP_LOGI(TAG, "OTA_RESTORE:%s,%d", ret == ESP_OK?"OK":"FAIL", ret);
    /* do not reboot in this function */
    return ret;
}
