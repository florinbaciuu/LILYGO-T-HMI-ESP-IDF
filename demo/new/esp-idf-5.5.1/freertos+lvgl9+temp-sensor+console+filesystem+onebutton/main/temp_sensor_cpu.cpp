extern "C" {
#include "driver/temperature_sensor.h"
#include "esp_system.h"
}
#include "temp_sensor_cpu.h"

/**
 * @brief temperature_sensor_config_t default constructor
 */
#define TEMPERATURE_SENSOR_CONFIG_DEFAULT_FLO(min, max)    \
    {                                                  \
        .range_min = min,                              \
        .range_max = max,                              \
        .clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT, \
        .flags = { .allow_pd = 1 },                    \
    }

void check_temp_once() {
    temperature_sensor_handle_t temp_handle = NULL;
    temperature_sensor_config_t config = TEMPERATURE_SENSOR_CONFIG_DEFAULT_FLO(20, 80);
    temperature_sensor_install(&config, &temp_handle);
    temperature_sensor_enable(temp_handle);
    float cpu_temperat;
    temperature_sensor_get_celsius(temp_handle, &cpu_temperat);
    printf("Chip temperature: %.2f °C\n", cpu_temperat);
    temperature_sensor_disable(temp_handle);
    temperature_sensor_uninstall(temp_handle);
}