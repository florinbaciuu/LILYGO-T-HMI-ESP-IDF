/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

/* tolower, islower, isxdigit, etc */
#include <ctype.h>
/* uint8_t */
#include <stdint.h>
/* size_t */
#include <stddef.h>
/* PRIu32 */
#include <inttypes.h>

#include "sdkconfig.h"

/* IPSTR,IP2STR from esp_netif component */
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_QA_UTILS_LOG_OUTPUT_ESP_LOG
#include "esp_log.h"
#else
#define LOG_RESET_COLOR ""
#define LOG_COLOR_I ""
#endif

#ifdef CONFIG_QA_TEST_LOG_TAG
#define QA_TEST_TAG CONFIG_QA_TEST_LOG_TAG
#else
#define QA_TEST_TAG "test"
#endif

#define _LOG_FORMAT(tag, format)  LOG_COLOR_I tag ": " format LOG_RESET_COLOR "\n"
#define QA_TEST_LOG(format, ...)  qa_utils_log_writev(_LOG_FORMAT(QA_TEST_TAG, format), ##__VA_ARGS__)
#define QA_TEST_ERROR(format, ...)  QA_TEST_LOG("@EW:"format, ##__VA_ARGS__)
/* log result: OK,FAIL */
#define QA_TEST_LOG_RES(res, format, ...)                     \
do {                                                          \
    if (res == 0) {                                           \
        QA_TEST_LOG(format",OK.", ##__VA_ARGS__);             \
    }else {                                                   \
        QA_TEST_LOG(format",FAIL.(ret=%d)", ##__VA_ARGS__, (int)res);  \
    }                                                         \
} while (0)
/* for esp console command finish */
#define QA_TEST_CMD_DONE(err, format, ...) QA_TEST_LOG_RES(err, "DONE."format, ##__VA_ARGS__)


#define QA_TEST_ASSERT(cond)                                  \
do {                                                          \
    bool __res = (cond);                                      \
    if (!__res) {                                             \
        QA_TEST_ERROR("%s:%d, Assertion `%s` Failed", __FILE__, __LINE__, #cond); \
        abort();                                              \
    }                                                         \
} while (0)


#define QA_UTILS_FREE(ptr)                              \
do {                                                    \
    void *__point = (void *)(ptr);                      \
    qa_utils_free(__point, __FILE__, __LINE__);         \
    ptr = NULL;                                         \
} while (0)

#define QA_UTILS_MALLOC(size)                           \
({                                                      \
    void *__p;                                          \
    __p = qa_utils_malloc(size, __FILE__, __LINE__);    \
    (void *)__p;                                        \
})

#define QA_UTILS_CALLOC(cnt, size)                           \
({                                                           \
    void *__p;                                               \
    __p = qa_utils_calloc(cnt, size, __FILE__, __LINE__);    \
    (void *)__p;                                             \
})


/**
 * printf or log write with highest level, no tag
 */
void qa_utils_log_writev(const char* format, ...) __attribute__((format(printf, 1, 2)));

/**
 * convert str to int
 */
int qa_utils_str2int(const char* str);

/**
 * convert hex str (0xAABBCCDD010203) to data (uint8_t*)
 */
int qa_utils_hex2data(const char* str, uint8_t* data, int data_len);

/**
 * convert str to mac (uint8*)
 */
int qa_utils_str2mac(const char* str, uint8_t* mac, int mac_len);

/**
 * convert mac to str xx:xx:xx:xx:xx:xx, recommended to use MACSTR from esp_mac.h for log output.
 */
int qa_utils_mac2str(const uint8_t* mac, int mac_len, char* str, int str_len);

/**
 * convert string to ip (uint32_t network byte order), can use esp_netif_str_to_ip4 for esp_ip_addr_t
 */
int qa_utils_str2ip(const char* str, uint32_t* ip);

/**
 * convert ip (uint32_t network byte order) to str, recommended to use IPSTR from esp_netif component for log output.
 */
int qa_utils_ip2str(uint32_t ip, char* str, int str_len);

/**
 * malloc wrapper
 */
void *qa_utils_malloc(size_t size, const char* file, int line);

/**
 * calloc wrapper
 */
void *qa_utils_calloc(int count, size_t size, const char* file, int line);

/**
 * free wrapper
 */
void qa_utils_free(void *ptr, const char* file, int line);



#ifdef __cplusplus
}
#endif
