include($ENV{IDF_PATH}/tools/cmake/version.cmake)

set(srcs  "src/system_cmd.c"
          "src/info_cmd.c"
          "src/heap_cmd.c"
          "src/mac_cmd.c"
          "src/restart_cmd.c"
          "src/reg_cmd.c")
set(priv_requires console spi_flash esp_hw_support)

if (CONFIG_SYS_CMD_SUPPORT_NVS)
    list(APPEND srcs "src/nvs_cmd.c")
endif()

if (CONFIG_SYS_CMD_SUPPORT_DSLEEP)
    list(APPEND srcs "src/dsleep_cmd.c")
endif()

if(${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR} VERSION_GREATER 4.4)
    idf_component_register(SRCS "${srcs}"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES ${priv_requires})

    if(CONFIG_SYS_CMD_SUPPORT_NVS)
        idf_component_optional_requires(PRIVATE nvs_flash)
    endif()
else()
    idf_component_register(SRCS "${srcs}"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES ${priv_requires} nvs_flash)
endif()



if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.0)
    target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE SYS_CMD_TAG="${CONFIG_SYS_CMD_LOG_MODULE}")
