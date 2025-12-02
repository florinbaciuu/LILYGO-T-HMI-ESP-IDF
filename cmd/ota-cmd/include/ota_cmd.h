/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register command: ota (using http[s])
 */
void ota_cmd_register_ota(void);

/**
 * @brief Register command: simpleota
 */
void ota_cmd_register_simpleota(void);

/* Deprecated */
#define app_register_ota_commands ota_cmd_register_ota

#ifdef __cplusplus
}
#endif
