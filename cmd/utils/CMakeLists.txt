include($ENV{IDF_PATH}/tools/cmake/version.cmake)

set(srcs  "src/ip_addr.c"
          "src/lib.c"
          "src/log.c"
          "src/malloc.c")
set(priv_requires esp_netif console log)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES ${priv_requires})

target_compile_options(${COMPONENT_LIB} PRIVATE "-Os")

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.0)
    target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
endif()
