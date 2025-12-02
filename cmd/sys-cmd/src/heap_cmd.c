/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"

static const char* TAG = SYS_CMD_TAG;

typedef enum {
    HEAP_ACTION_MALLOC,
    HEAP_ACTION_FREE,
    HEAP_ACTION_QUERY,
} heap_action_t;

typedef struct {
    struct arg_str *action;
    struct arg_int *index;
    struct arg_int *size;
    struct arg_end *end;
} heap_args_t;
static heap_args_t s_heap_args;

#define CMD_MAX_MALLOC_COUNT CONFIG_SYS_CMD_HEAP_MALLOC_MAX_COUNT
static void* s_malloc_heaps[CMD_MAX_MALLOC_COUNT] = {NULL};

static heap_action_t str_to_heap_action(const char* str)
{
    if (!strcmp(str, "malloc")) {
        return HEAP_ACTION_MALLOC;
    } else if(!strcmp(str, "free")) {
        return HEAP_ACTION_FREE;
    } else {
        return HEAP_ACTION_QUERY;
    }
}

static int cmd_do_heap(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &s_heap_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_heap_args.end, argv[0]);
        return 1;
    }

    heap_action_t action = HEAP_ACTION_QUERY;
    if (s_heap_args.action->count > 0 ) {
        action = str_to_heap_action(s_heap_args.action->sval[0]);
    }
    int index = -1;
    if (s_heap_args.index->count > 0 ) {
        index = s_heap_args.index->ival[0];
    }
    if (index >= CMD_MAX_MALLOC_COUNT || index < -1) {
        ESP_LOGE(TAG, "Index out of range!");
        return 1;
    }
    switch (action) {
        case HEAP_ACTION_MALLOC:
            if (index == -1) index = 0;
            if (s_malloc_heaps[index] != NULL) {
                ESP_LOGW(TAG, "Force free %p before malloc new heap", s_malloc_heaps[index]);
                s_malloc_heaps[index] = NULL;
            }
            if (s_heap_args.size->count == 0) {
                ESP_LOGE(TAG, "size must be given when using heap malloc");
            }
            s_malloc_heaps[index] = malloc(s_heap_args.size->ival[0]);
            if (s_malloc_heaps[index] == NULL) {
                ESP_LOGE(TAG, "malloc failed (size:%d)", s_heap_args.size->ival[0]);
            }else {
                ESP_LOGI(TAG, "HEAP_MALLOC:0x%p (size:%d)", s_malloc_heaps[index], s_heap_args.size->ival[0]);
            }
            break;
        case HEAP_ACTION_FREE:
            for (int i = 0; i < CMD_MAX_MALLOC_COUNT; i ++) {
                if (s_malloc_heaps[i] == NULL) continue;
                if (index == -1 || i == index) {
                    ESP_LOGI(TAG, "HEAP_FREE:0x%p", s_malloc_heaps[i]);
                    free(s_malloc_heaps[i]);
                    s_malloc_heaps[i] = NULL;
                }
            }
            break;
        case HEAP_ACTION_QUERY:
#if 1
            /* compatible with old version script for a while */
            ESP_LOGI(TAG, "min heap size: %d", esp_get_minimum_free_heap_size());
            ESP_LOGI(TAG, "MIN_HEAPSIZE:%d", esp_get_minimum_free_heap_size());
#endif
            /* compatible with SSC: "+FREEHEAP:1234" */
            ESP_LOGI(TAG, "CURRENT_FREEHEAP:%d", esp_get_free_heap_size());
            ESP_LOGI(TAG, "INTERNAL_FREE:%d", esp_get_free_internal_heap_size());
            ESP_LOGI(TAG, "MIN_FREE:%d", esp_get_minimum_free_heap_size());
            break;
        default:
            ESP_LOGE(TAG, "Unknown action.");
            break;
    }
    return 0;
}

void sys_cmd_register_heap(void)
{
    s_heap_args.action = arg_str0(NULL, NULL, "<action>", "query, malloc, free.");
    s_heap_args.index = arg_int0("i", "index", "<int>", "heap malloc/free index (0~4). default: 0 for malloc, -1(all) for free");
    s_heap_args.size = arg_int0(NULL, "size", "<bytes>", "malloc size in bytes.");
    s_heap_args.end = arg_end(1);
    const esp_console_cmd_t heap_cmd = {
        .command = "heap",
        .help = "Heap command.",
        .hint = NULL,
        .func = &cmd_do_heap,
        .argtable = &s_heap_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&heap_cmd) );
}
