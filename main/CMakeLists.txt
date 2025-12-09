

cmake_minimum_required(VERSION 3.5)

# Set usual component variables
set( app_sources "main.cpp" "temp_sensor_cpu.cpp" "rtos.cpp" )
set( app_include_dirs "." "" )
set( app_requires button cmake_utilities coremark esp_lcd_touch esp_lcd_touch_xpt2046 esp_lv_fs esp_lvgl_port esp_mmap_assets fmt freertos-cpp littlefs lvgl )
set( app_priv_requires ${app_requires} esp_bootloader_format nvs_flash esp_wifi esp_rom driver fatfs spi_flash esp_driver_usb_serial_jtag esp_system heap
                    esp_common esp_psram esp_hw_support console )

# Set my components
# These are components that are not part of the IDF 
# but are used in this project.
# Put this components in /lib after set 
# set(EXTRA_COMPONENT_DIRS lib) in root/CMakeLists.txt
set(my_components   one-cli-v005
                    onebutton-v001
                    filesystem-v003
)


idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS ${app_include_dirs}
    REQUIRES ${app_requires} ${my_components}
    PRIV_REQUIRES ${app_priv_requires}
    WHOLE_ARCHIVE
)

add_compile_options(-Wno-error)

idf_build_get_property(components BUILD_COMPONENTS)
message(STATUS "@@@ ALL COMPONENTS seen from MAIN  = ${components}")

# target_compile_options(${COMPONENT_LIB} PUBLIC
#     -Wno-error
#     -Wno-error=cpp
#     -Wno-error=type-limits
#     -Wno-unused-variable
#     -Wno-unused-function
#     -Wno-error=deprecated-declarations
# )

# set_source_files_properties(
#     main.cpp rtos.cpp temp_sensor_cpu.cpp
#     PROPERTIES COMPILE_FLAGS "-Wno-error=cpp -Wno-error "
# )


# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
# littlefs_create_partition_image(littlefs littlefs_data FLASH_IN_PROJECT)
# spiffs_create_partition_image(spiffs spiffs_data FLASH_IN_PROJECT)
