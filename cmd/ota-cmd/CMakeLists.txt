include($ENV{IDF_PATH}/tools/cmake/version.cmake)

set(srcs  "src/ota_cmd.c"
          "src/ota_common.c"
          "src/simpleota_cmd.c")

set(priv_requires console app_update esp_http_client esp_https_ota)

# version >= v4.4
if(${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR} VERSION_GREATER 4.3)
    list(APPEND priv_requires esp_timer)
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES ${priv_requires})

# version >= v5.0
if(${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR} VERSION_GREATER 4.4)
    if(NOT CONFIG_OTA_CMD_USE_ESP_HTTPS_OTA)
        idf_component_optional_requires(PRIVATE esp_https_ota)
    endif()
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.0)
    target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
endif()
