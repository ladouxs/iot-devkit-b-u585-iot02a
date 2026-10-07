cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/U585AIIQ/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(utilities)

project(WiFi_template
    LANGUAGES C ASM
)

set(THREADX_ARCH cortex_m33)
set(THREADX_TOOLCHAIN gnu)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ---------------------------------------------------------------------------
# STM32U585 configuration
# ---------------------------------------------------------------------------

add_executable(${PROJECT_NAME})

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_arm_post_build_with_usage(${PROJECT_NAME} 2048 786)

add_compile_definitions(NX_BSD_ENABLE_NATIVE_API)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        USE_HAL_DRIVER
        STM32U585xx
)

set(NXD_ENABLE_BSD ON CACHE BOOL "Enable NetX Duo BSD compatibility layer" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/U585AIIQ)

target_sources(${PROJECT_NAME}
    PRIVATE
        startup_stm32u585xx.s
)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
        m
)

# WiFi compile definitions: require WIFI_SSID and propagate definitions to all subdirectories
set(WIFI_SSID "" CACHE STRING "WiFi SSID for MXCHIP module")
set(WIFI_PASSWORD "" CACHE STRING "WiFi password for MXCHIP module")

if(NOT DEFINED WIFI_SSID OR WIFI_SSID STREQUAL "")
    message(FATAL_ERROR "WIFI_SSID must be provided at configure time. Example: -DWIFI_SSID=\"MySSID\"")
endif()

add_compile_definitions(WIFI_SSID="${WIFI_SSID}")
if(DEFINED WIFI_PASSWORD)
    add_compile_definitions(WIFI_PASSWORD="${WIFI_PASSWORD}")
else()
    add_compile_definitions(WIFI_PASSWORD="")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    NX_BSD_ENABLE_NATIVE_API
    WIFI_SSID="${WIFI_SSID}"
    WIFI_PASSWORD="${WIFI_PASSWORD}"
)

add_subdirectory(${CMAKE_SOURCE_DIR}/Core)
