cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(SHARED_LIB_DIR ${CMAKE_SOURCE_DIR}/shared/lib)

include(utilities)

project(THOR-EV LANGUAGES C CXX ASM)
set (PROJECT_NAME "THOR-EV")

set(THREADX_ARCH cortex_m33)
set(THREADX_TOOLCHAIN gnu)

add_compile_options(
    -mcpu=cortex-m33
    -mthumb
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
)

add_link_options(
    -mcpu=cortex-m33
    -mthumb
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
)

add_subdirectory(${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/app)

